#include <stdexcept>
#include <sstream>
#include <appfw/appfw.h>
#include <appfw/command_line.h>

bool appfw::CommandLine::isFlagSet(const std::string &tag, char shorttag) {
    return m_ShortFlagList.find(shorttag) != m_ShortFlagList.end() || m_FlagList.find(tag) != m_FlagList.end();
}

bool appfw::CommandLine::doesArgHaveValue(const std::string &tag) { return m_Args.find(tag) != m_Args.end(); }

std::string appfw::CommandLine::getArgString(const std::string &tag, const std::optional<std::string> &defval) {
    auto it = m_Args.find(tag);

    if (it != m_Args.end()) {
        return it->second;
    } else {
        if (defval) {
            return *defval;
        } else {
            throw std::runtime_error(fmt::format("argument {} is required", tag));
        }
    }
}

int appfw::CommandLine::getArgInt(const std::string &tag, const std::optional<int> &defval) {
    auto it = m_Args.find(tag);

    if (it != m_Args.end()) {
        int val;

        if (!appfw::convertStringToVal(it->second, val)) {
            throw std::runtime_error(fmt::format("argument {} is not an integer", tag));
        }

        return val;
    } else {
        if (defval) {
            return *defval;
        } else {
            throw std::runtime_error(fmt::format("argument {} is required", tag));
        }
    }
}

float appfw::CommandLine::getArgFloat(const std::string &tag, const std::optional<float> &defval) {
    auto it = m_Args.find(tag);

    if (it != m_Args.end()) {
        float val;

        if (!appfw::convertStringToVal(it->second, val)) {
            throw std::runtime_error(fmt::format("argument {} is not a float", tag));
        }

        return val;
    } else {
        if (defval) {
            return *defval;
        } else {
            throw std::runtime_error(fmt::format("argument {} is required", tag));
        }
    }
}

void appfw::CommandLine::execCommands(bool execNow) {
    for (auto &i : m_ExecCmds) {
        getConsole().command(i);
    }

    if (execNow) {
        getConsole().processAllCommands();
    }
}

void appfw::CommandLine::parseCommandLine(int argc, const char *const *argv, bool logArgs) {
    AFW_ASSERT(argc >= 0);
    
    if (argc == 0) {
        return;
    }

    m_CmdName = argv[0];

    // Print arguments
    if (logArgs) {
        std::ostringstream args;

        for (int i = 1; i < argc; i++) {
            if (i != 1) {
                args << ' ';
            }
            
            std::string arg = argv[i];

            if (arg.find(' ') != arg.npos) {
                args << '"' << arg << '"';
            } else {
                args << arg;
            }
        }

        printi("Command line: {}", args.str());
    }

    // Parse arguments
    bool areNextArgsPositional = false;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (areNextArgsPositional) {
            // Save as a pos arg
            m_PosArgs.push_back(arg);
            continue;
        }

        if (arg[0] == '-' && arg[1] == '-') {
            // Full tag
            if (!arg[2]) {
                // Only --, assume everything after this arg is a pos arg
                areNextArgsPositional = true;
                continue;
            }

            std::string argstr = arg;

            // Check if next arg is a value
            if (i + 1 < argc && argv[i + 1][0] != '-' && argv[i + 1][0] != '+') {
                // A value
                auto it = m_Args.find(argstr);
                if (it == m_Args.end()) {
                    m_Args.insert({argstr, std::string(argv[i + 1])});
                } else {
                    throw std::runtime_error(fmt::format("command line argument {} given multiple times", i));
                }

                i++;
            } else {
                // A flag
                m_FlagList.insert(argstr);
            }

        } else if (arg[0] == '-') {
            // Short tag
            std::string argstr = arg + 1;

            // Set all letters after dash
            for (char c : argstr) {
                m_ShortFlagList.insert(c);
            }
        } else if (arg[0] == '+') {
            // Command to execute
            auto cmds = CmdString::parse(arg + 1);
            
            for (auto &cmd : cmds) {
                m_ExecCmds.push_back(std::move(cmd));
            }
        } else {
            // Positional argument
            m_PosArgs.push_back(arg);
        }

        // 'i' may have been changed to one or more next args, don't use it here
    }
}
