#ifndef APPFW_CONSOLE_CONSOLE_SYSTEM_H
#define APPFW_CONSOLE_CONSOLE_SYSTEM_H
#include <map>
#include <set>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <appfw/console/con_item.h>
#include <appfw/console/con_msg.h>
#include <appfw/cmd_buffer.h>
#include <appfw/utils.h>

namespace appfw {

class ConsoleSystem;

/**
 * An interface for any console output dialog.
 */
class IConsoleReceiver {
public:
    virtual ~IConsoleReceiver() = default;

    /**
     * Called when this receiver was registered.
     * @arg conSystem Console system this was added to.
     */
    virtual void onAdd(ConsoleSystem *pConSys) = 0;

    /**
     * Called when this receiver was removed.
     * @arg conSystem Console system this was removed from.
     */
    virtual void onRemove(ConsoleSystem *pConSys) = 0;

    /**
     * Called to display a string to the console.
     */
    virtual void print(const ConMsgInfo &info, std::string_view msg) = 0;

    /**
     * Returns whether the receiver's print function can be called from multiple threads.
     * Locking is provided by console system (print won't be called from two threads at the same time)
     */
    virtual bool isThreadSafe() = 0;
};

class ConsoleSystem : NoMove {
public:
    ConsoleSystem();
    ~ConsoleSystem();

    /**
     * Prints a string to the console. Thread-safe.
     */
    void print(const ConMsgInfo &info, std::string_view msg);

    /**
     * Prints a string to the console. Thread-safe.
     */
    void print(const ConMsgInfo &info, std::string &&msg);

    /**
     * Returns whether console output is thread-safe.
     * If false, messages from other threads will be delayed until next tick.
     */
    bool isThreadSafeOutput();

    /**
     * Finds an item by name.
     * @param   name    Name of the item.
     * @param   type    Type of the item (or Any).
     * @return Found item or nullptr.
     */
    ConItemBase *findItem(std::string_view name, ConItemType type = ConItemType::Any);

    /**
     * Finds a console variable by name.
     * @param   name    Name of the convar.
     * @return Found item or nullptr.
     */
    inline ConVarBase *findCvar(const std::string &name) {
        return static_cast<ConVarBase *>(findItem(name, ConItemType::ConVar));
    }

    /**
     * Finds a console command by name.
     * @param   name    Name of the command.
     * @return Found item or nullptr.
     */
    inline ConVarBase *findCommand(const std::string &name) {
        return static_cast<ConVarBase *>(findItem(name, ConItemType::ConCommand));
    }

    /**
     * Returns map of all items, ordered alphabetically.
     */
    const auto &getItemMap() { return m_ConItems; }

    /**
     * Registers an item.
     */
    void registerConItem(ConItemBase *pItem);

    /**
     * Unregisters an item.
     */
    void unregisterConItem(ConItemBase *pItem);

    /**
     * Parses `cmd` and adds it to the bottom of the queue.
     */
    void command(const std::string &cmd);

    /**
     * Adds `cmd` to the bottom of the queue.
     */
    void command(const CmdString &cmd);

    /**
     * Executes specified command immediately.
     */
    void commandNow(const std::string &cmd);

    /**
     * Executes specified command immediately.
     */
    void commandNow(const CmdString &cmd);

    /**
     * Executes a command at the top of the buffer.
     */
    void processCommand();

    /**
     * Executes all commands available in the buffer at the moment of call.
     */
    void processAllCommands();

    /**
     * Prints all pending messages from other threads.
     */
    void processMsgQueue();

    /**
     * Adds a console receiver to this console system.
     */
    void addConsoleReceiver(IConsoleReceiver *pRecv);

    /**
     * Removes a previously added console receiver.
     */
    void removeConsoleReceiver(IConsoleReceiver *pRecv);

    /**
     * Prints saved previous messages to a specified receiver.
     */
    void printPreviousMessages(IConsoleReceiver *pRecv);

private:
    class RingBuffer;

    std::unique_ptr<RingBuffer> m_pMsgBuf;
    std::map<std::string, ConItemBase *, std::less<>> m_ConItems;
    std::set<IConsoleReceiver *> m_RecvList;
    CmdBuffer m_CmdBuffer;

    // Output thread safety
    std::queue<ConMsg> m_ThreadedMsgQueue;
    std::thread::id m_MainThread;
    std::mutex m_OutputMutex;
    bool m_bThreadSafeOutput = true;

    void updateThreadSafetyState();
};

} // namespace appfw

#endif
