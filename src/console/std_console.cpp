#include <iostream>
#include <appfw/console/std_console.h>

#if PLATFORM_WINDOWS
#include <appfw/windows.h>
#endif

appfw::StdConsole::StdConsole(TermInputMethod inputMethod) {
    m_InputMethod = inputMethod;

    if (m_InputMethod == TermInputMethod::Enable) {
        m_pSyncData = std::make_shared<SyncData>();
        m_pSyncData->bIsValid = true;
        m_Thread = std::thread(threadWorker, this, m_pSyncData);
        m_Thread.detach();
    }
}

appfw::StdConsole::~StdConsole() {
    if (m_InputMethod == TermInputMethod::Enable) {
        std::lock_guard<std::mutex> lock(m_pSyncData->mutex);
        m_pSyncData->bIsValid = false;
    }
}

void appfw::StdConsole::onAdd(ConsoleSystem *conSys) {
    AFW_ASSERT(!m_pConSys);
    m_pConSys = conSys;
}

void appfw::StdConsole::onRemove([[maybe_unused]] ConsoleSystem *conSys) {
    AFW_ASSERT(m_pConSys == conSys);
    m_pConSys = nullptr;
}

void appfw::StdConsole::print(const ConMsgInfo &info, std::string_view msg) {
    setColor(info.color != ConMsgColor::Default ? info.color : getMsgTypeColor(info.type));
    std::cout << msg << "\n";
    setColor(ConMsgColor::Default);
}

bool appfw::StdConsole::isThreadSafe() {
    return true;
}

appfw::TermInputMethod appfw::StdConsole::getInputMethod() {
    return m_InputMethod;
}

void appfw::StdConsole::tick() {
    if (m_InputMethod == TermInputMethod::Enable) {
        std::lock_guard<std::mutex> lock(m_pSyncData->mutex);
        if (!m_CmdQueue.empty()) {
            const std::string &cmd = m_CmdQueue.front();
            auto msgInfo = ConMsgInfo().setType(ConMsgType::Input).setTag("stdin");
            m_pConSys->print(msgInfo, "> " + cmd);
            m_pConSys->command(cmd);
            m_CmdQueue.pop();
        }
    }
}

void appfw::StdConsole::threadWorker(StdConsole *t, std::shared_ptr<SyncData> pSyncData) {
    for (;;) {
        std::string input;
        std::getline(std::cin, input);

        std::lock_guard<std::mutex> lock(pSyncData->mutex);

        if (!pSyncData->bIsValid) {
            // StdConsole no longer exists
            return;
        }

        t->m_CmdQueue.push(input);
    }
}

#if PLATFORM_WINDOWS
void appfw::StdConsole::setColor(ConMsgColor color) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD colorword = (color != ConMsgColor::Default) ? (WORD)color : (WORD)ConMsgColor::Grey;
    SetConsoleTextAttribute(h, colorword);
}
#else
void appfw::StdConsole::setColor(ConMsgColor color) {
    // Do nothing
}
#endif