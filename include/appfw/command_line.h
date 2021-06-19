#ifndef APPFW_COMMAND_LINE_H
#define APPFW_COMMAND_LINE_H
#include <set>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <appfw/cmd_string.h>
#include <appfw/utils.h>

namespace appfw {

class CommandLine {
public:
    /**
     * Retruns whether a flag is set.
     * @param   tag         Full tag: "--full-tag-name"
     * @param   shorttag    Short tag: 'f', in console -f
     */
    bool isFlagSet(const std::string &tag, char shorttag = '\0');

    /**
     * Returns whether argument was given with a value in the command line.
     */
    bool doesArgHaveValue(const std::string &tag);

    /**
     * Returns value of argument as string. If argument and defval are not given, throws an exception.
     */
    std::string getArgString(const std::string &tag, const std::optional<std::string> &defval = std::nullopt);

    /**
     * Returns value of argument as string. If argument and defval are not given, throws an exception.
     */
    int getArgInt(const std::string &tag, const std::optional<int> &defval = std::nullopt);

    /**
     * Returns value of argument as string. If argument and defval are not given, throws an exception.
     */
    float getArgFloat(const std::string &tag, const std::optional<float> &defval = std::nullopt);

    /**
     * Returns the command name this executable was called with (0th argument).
     */
    inline const std::string &getCommandName() { return m_CmdName; }

    /**
     * Returns list of positional arguments (args without --, - or +).
     */
    inline const std::vector<std::string> &getPosArgs() { return m_PosArgs; }

    /**
     * Returns list of commands that need to be executed.
     */
    inline const std::vector<CmdString> &getExecCommands() { return m_ExecCmds; }

    /**
     * Executes commands with +.
     * @param   execNow     Execute whole command buffer at the end
     */
    void execCommands(bool execNow = true);

    /**
     * Parses given command line into this object.
     */
    void parseCommandLine(int argc, const char *const *argv, bool logArgs = true);

private:
    std::string m_CmdName;
    std::set<std::string> m_FlagList;
    std::set<char> m_ShortFlagList;
    std::map<std::string, std::string> m_Args;
    std::vector<CmdString> m_ExecCmds;
    std::vector<std::string> m_PosArgs;
};

} // namespace appfw

#endif
