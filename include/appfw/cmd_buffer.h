#ifndef APPFW_COMMAND_BUFFER_H
#define APPFW_COMMAND_BUFFER_H
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>

#include <appfw/utils.h>
#include <appfw/cmd_string.h>

namespace appfw {

/**
 * A thread-safe queue of commands.
 */
class CmdBuffer {
public:
    using CommandHandler = std::function<void(const CmdString &args)>;

    CmdBuffer() = default;
    CmdBuffer(const CommandHandler &handler);

    /**
     * Parses `cmd` and adds it to the bottom of the queue.
     */
    void append(const std::string &cmd);

    /**
     * Adds `cmd` to the bottom of the queue.
     */
    void append(const CmdString &cmd);

    /**
     * Adds `cmds` to the bottom of the queue.
     */
    void append(const std::vector<CmdString> &cmds);

    /**
     * Executes a command at the top of the buffer.
     */
    void executeOnce();

    /**
     * Executes all commands available in the buffer at the moment of call.
     */
    void executeAll();

    /**
     * Returns number of commands waiting to be executed.
     */
    size_t getCommandCount();

    /**
     * Sets a function that handles a command.
     * @param   handler     A function to execute a command.
     */
    void setCommandHandler(const CommandHandler &handler);

private:
    std::mutex m_Mutex;
    std::queue<CmdString> m_CmdQueue;
    CommandHandler m_Handler;
};

} // namespace appfw

#endif
