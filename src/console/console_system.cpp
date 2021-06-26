#include <appfw/appfw.h>
#include <appfw/console/console_system.h>
#include <appfw/dbg.h>

//----------------------------------------------------------------
// RingBuffer
//----------------------------------------------------------------
class appfw::ConsoleSystem::RingBuffer {
public:
    static constexpr size_t MAX_BUFFER_SIZE = 64;

    RingBuffer();

    //! Adds a message to the end of the buffer
    void push(const ConMsgInfo &info, std::string_view msg);

    //! Moves a message to the end of the buffer
    const ConMsg &push(const ConMsgInfo &info, std::string &&msg);

    //! Moves a message to the end of the buffer
    const ConMsg &push(ConMsg &&msg);

    //! Returns the number of saved messages
    size_t size();

    //! Prints ring buffer contents into the receiver
    void printPreviousMessages(IConsoleReceiver *pRecv);

private:
    std::vector<ConMsg> m_Buf;
    size_t m_uPtr = 0;
    size_t m_uSize = 0;
    std::mutex m_Mutex;
};

appfw::ConsoleSystem::RingBuffer::RingBuffer() {
    m_Buf.resize(MAX_BUFFER_SIZE);
}

void appfw::ConsoleSystem::RingBuffer::push(const ConMsgInfo &info, std::string_view msg) {
    ConMsg m = {info, std::string(msg)};
    push(std::move(m));
}

const appfw::ConMsg &appfw::ConsoleSystem::RingBuffer::push(const ConMsgInfo &info, std::string &&msg) {
    ConMsg m = {info, std::move(msg)};
    return push(std::move(m));
}

const appfw::ConMsg &appfw::ConsoleSystem::RingBuffer::push(ConMsg &&msg) {
    std::lock_guard lock(m_Mutex);
    m_Buf[m_uPtr] = std::move(msg);
    ConMsg &newmsg = m_Buf[m_uPtr];
    m_uPtr++;

    if (m_uPtr == m_Buf.size()) {
        m_uPtr = 0;
    }

    if (m_uSize < m_Buf.size()) {
        m_uSize++;
    }

    return newmsg;
}

size_t appfw::ConsoleSystem::RingBuffer::size() {
    return m_uSize;
}

void appfw::ConsoleSystem::RingBuffer::printPreviousMessages(IConsoleReceiver *pRecv) {
    std::lock_guard lock(m_Mutex);

    if (m_uSize < m_Buf.size()) {
        for (size_t i = 0; i < m_uSize; i++) {
            auto &[info, msg] = m_Buf[i];
            pRecv->print(info, msg);
        }
    } else {
        for (size_t i = m_uPtr; i < m_uSize; i++) {
            auto &[info, msg] = m_Buf[i];
            pRecv->print(info, msg);
        }

        for (size_t i = 0; i < m_uPtr; i++) {
            auto &[info, msg] = m_Buf[i];
            pRecv->print(info, msg);
        }
    }
}

//----------------------------------------------------------------
// ConsoleSystem
//----------------------------------------------------------------
appfw::ConsoleSystem::ConsoleSystem() {
    m_pMsgBuf = std::make_unique<RingBuffer>();
    updateThreadSafetyState();

    m_CmdBuffer.setCommandHandler([this](const CmdString &cmd) { commandNow(cmd); });
    m_MainThread = std::this_thread::get_id();

    // Register pending items
    auto &pending = ConItemBase::getPendingItems();
    for (auto item : pending) {
        registerConItem(item);
    }
    pending.clear();
}

appfw::ConsoleSystem::~ConsoleSystem() = default;

void appfw::ConsoleSystem::print(const ConMsgInfo &info, std::string_view msg) {
    std::lock_guard lock(m_OutputMutex);
    m_pMsgBuf->push(info, msg);

    if (m_bThreadSafeOutput || std::this_thread::get_id() == m_MainThread) {
        // Output right now
        for (IConsoleReceiver *pRecv : m_RecvList) {
            pRecv->print(info, msg);
        }
    } else {
        // Put into the queue
        m_ThreadedMsgQueue.push({info, std::string(msg)});
    }
}

void appfw::ConsoleSystem::print(const ConMsgInfo &info, std::string &&msg) {
    std::lock_guard lock(m_OutputMutex);
    const ConMsg &conMsg = m_pMsgBuf->push(info, std::move(msg));

    if (m_bThreadSafeOutput || std::this_thread::get_id() == m_MainThread) {
        // Output right now
        for (IConsoleReceiver *pRecv : m_RecvList) {
            pRecv->print(info, conMsg.second);
        }
    } else {
        // Put a copy into the queue
        m_ThreadedMsgQueue.push(conMsg);
    }
}

bool appfw::ConsoleSystem::isThreadSafeOutput() {
    return m_bThreadSafeOutput;
}

appfw::ConItemBase *appfw::ConsoleSystem::findItem(std::string_view name, ConItemType type) {
    auto it = m_ConItems.find(name);

    if (it == m_ConItems.end()) {
        return nullptr;
    }

    ConItemType itemType = it->second->getType();

    if (type == ConItemType::Any || itemType == type) {
        return it->second;
    } else {
        return nullptr;
    }
}

void appfw::ConsoleSystem::registerConItem(ConItemBase *pItem) {
    m_ConItems.insert({std::string(pItem->getName()), pItem});
}

void appfw::ConsoleSystem::unregisterConItem(ConItemBase *pItem) {
    auto it = m_ConItems.find(pItem->getName());
    AFW_ASSERT(it != m_ConItems.end());
    AFW_ASSERT(it->second == pItem);
    m_ConItems.erase(it);
}

void appfw::ConsoleSystem::command(const std::string &cmd) {
    m_CmdBuffer.append(cmd);
}

void appfw::ConsoleSystem::command(const CmdString &cmd) {
    m_CmdBuffer.append(cmd);
}

void appfw::ConsoleSystem::commandNow(const std::string &cmd) {
    auto cmds = CmdString::parse(cmd);

    for (auto &i : cmds) {
        commandNow(i);
    }
}

void appfw::ConsoleSystem::commandNow(const CmdString &cmd) {
    AFW_ASSERT(!cmd.empty());
    ConItemBase *pItem = findItem(cmd[0]);

    if (!pItem) {
        printe("Unknown command: {}", cmd[0]);
        return;
    }

    ConItemType type = pItem->getType();

    if (type == ConItemType::ConVar) {
        ConVarBase *pCvar = static_cast<ConVarBase *>(pItem);

        if (cmd.size() == 1) {
            printn("    {} = \"{}\" ({}) {}", pCvar->getName(),
                     pCvar->getStringValue(), pCvar->getVarTypeAsString(), pCvar->isLocked() ? "(locked)" : "");
            printi("{}", pCvar->getDescr());
        } else {
            VarSetResult result = pCvar->setStringValue(cmd[1]);

            switch (result) {
            case VarSetResult::Success:
                break;
            case VarSetResult::InvalidString:
                printe("Set cvar {} failed: invalid string value.", pCvar->getName());
                break;
            case VarSetResult::CallbackRejected:
                printe("Set cvar {} failed: value rejected by callback.", pCvar->getName());
                break;
            case VarSetResult::Locked:
                printe("Set cvar {} failed: cvar is locked.", pCvar->getName());
                break;
            }
        }
    } else if (type == ConItemType::ConCommand) {
        ConCommand *pCmd = static_cast<ConCommand *>(pItem);
        pCmd->execute(cmd);
    } else {
        AFW_ASSERT_MSG(false, "Unknown console item type");
    }
}

void appfw::ConsoleSystem::processCommand() {
    m_CmdBuffer.executeOnce();
}

void appfw::ConsoleSystem::processAllCommands() {
    m_CmdBuffer.executeAll();
}

void appfw::ConsoleSystem::processMsgQueue() {
    std::lock_guard<std::mutex> lock(m_OutputMutex);
    AFW_ASSERT_MSG(m_ThreadedMsgQueue.size() <= 100, "Msg queue is unusually long (> 100)");
    m_MainThread = std::this_thread::get_id();

    while (!m_ThreadedMsgQueue.empty()) {
        auto &[info, msg] = m_ThreadedMsgQueue.front();
        
        for (IConsoleReceiver *pRecv : m_RecvList) {
            pRecv->print(info, msg);
        }

        m_ThreadedMsgQueue.pop();
    }
}

void appfw::ConsoleSystem::addConsoleReceiver(IConsoleReceiver *pRecv) {
    AFW_ASSERT_MSG(m_RecvList.find(pRecv) == m_RecvList.end(),
                   "pRecv has already been registered.");
    m_RecvList.insert(pRecv);
    pRecv->onAdd(this);

    updateThreadSafetyState();
}

void appfw::ConsoleSystem::removeConsoleReceiver(IConsoleReceiver *pRecv) {
    AFW_ASSERT_MSG(m_RecvList.find(pRecv) != m_RecvList.end(), "pRecv hasn't been registered.");
    pRecv->onRemove(this);
    m_RecvList.erase(pRecv);

    updateThreadSafetyState();
}

void appfw::ConsoleSystem::printPreviousMessages(IConsoleReceiver *pRecv) {
    m_pMsgBuf->printPreviousMessages(pRecv);
}

void appfw::ConsoleSystem::updateThreadSafetyState() {
    m_bThreadSafeOutput = true;

    for (auto pRecv : m_RecvList) {
        m_bThreadSafeOutput = m_bThreadSafeOutput && pRecv->isThreadSafe();
    }
}

//----------------------------------------------------------------

static void printItemInfo(appfw::ConItemBase *pItem) {
    using namespace appfw;

    ConItemType type = pItem->getType();

    if (type == ConItemType::ConVar) {
        ConVarBase *pCvar = static_cast<ConVarBase *>(pItem);
        printn("{} = \"{}\" ({}) {}", pCvar->getName(), pCvar->getStringValue(),
                       pCvar->getVarTypeAsString(), pCvar->isLocked() ? "(locked)" : "");
    } else {
        printn("{}", pItem->getName());
    }
    printi("{}", pItem->getDescr());
    printi("");
}

static ConCommand
    cmd_list("list", "Lists all available console items.\nOptional: add 'cvar' or 'cmd' to filter.",
           [](const appfw::CmdString &args) {
               using namespace appfw;

               ConItemType filter = ConItemType::Any;

               if (args.size() >= 2) {
                   if (args[1] == "cvar") {
                       filter = ConItemType::ConVar;
                   } else if (args[1] == "cmd") {
                       filter = ConItemType::ConCommand;
                   } else if (args[1] == "all") {
                       filter = ConItemType::Any;
                   } else {
                       printi("Usage: list [cvar|cmd|all]");
                       printi("{}", appfw::getConsole().findCommand("list")->getDescr());
                       return;
                   }
               }

               auto &map = appfw::getConsole().getItemMap();
               size_t count = 0;

               for (auto &i : map) {
                   if (filter == ConItemType::Any || filter == i.second->getType()) {
                       printItemInfo(i.second);
                       count++;
                   }
               }

               printi("Total: {} item{}", count, count == 1 ? "" : "s");
           });

static ConCommand cmd_help("help", "Shows info about a command.", [](const appfw::CmdString &args) {
                             using namespace appfw;

                             if (args.size() == 1) {
                                 printi("Usage: help <name>");
                                 return;
                             }

                             ConItemBase *pItem = appfw::getConsole().findItem(args[1]);

                             if (!pItem) {
                                 printe("Error: item \"{}\" not found", args[1]);
                                 return;
                             }

                             printItemInfo(pItem);
                         });

//----------------------------------------------------------------

static ConCommand cmd_debugLock("appfw_debug_lock_cvar", "Locks/unlocks a convar.",
                              [](const appfw::CmdString &args) {
                                  using namespace appfw;

                                  if (args.size() == 1) {
                                      printi("Usage: appfw_debug_lock_cvar <cvar name>");
                                      return;
                                  }

                                  ConVarBase *pCvar = appfw::getConsole().findCvar(args[1]);

                                  if (!pCvar) {
                                      printe("Error: cvar \"{}\" not found", args[1]);
                                      return;
                                  }

                                  pCvar->setLocked(!pCvar->isLocked());

                                  printi("Cvar \"{}\" is now {}", args[1],
                                           pCvar->isLocked() ? "locked" : "unlocked");
                              });

