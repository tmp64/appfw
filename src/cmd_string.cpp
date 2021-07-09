#include <appfw/cmd_string.h>

appfw::CmdString::CmdString(const std::string *itbeg, const std::string *itend) {
    size_t s = std::distance(itbeg, itend);
    m_Args.resize(s);
    std::copy(itbeg, itend, m_Args.begin());
}

std::string appfw::CmdString::toString() const {
    std::string s;

    if (!empty()) {
        s.push_back('"');
        s.append((*this)[0]);
        s.push_back('"');

        for (size_t i = 1; i < size(); i++) {
            s.push_back(' ');
            s.push_back('"');
            s.append((*this)[i]);
            s.push_back('"');
        }
    }

    return s;
}

std::vector<appfw::CmdString> appfw::CmdString::parse(std::string_view cmd) {
    if (cmd.empty()) {
        return std::vector<CmdString>();
    }

    std::vector<CmdString> strings;
    std::vector<std::string> args;
    size_t i = 0;

    // Skip spaces
    while (i < cmd.size() && cmd[i] == ' ')
        i++;

    constexpr size_t NO_ARG = std::numeric_limits<size_t>::max();

    // Parse the string
    size_t argStart = i;
    bool isInQuotes = false;
    for (; i <= cmd.size(); i++) {
        char c = (i == cmd.size()) ? '\0' : cmd[i];

        if (argStart == NO_ARG) {
            // Skip spaces
            if (c == ' ' || c == '\0') {
                continue;
            } else if (c == ';' || c == '\n') {
                // End of command
                if (!args.empty()) {
                    CmdString newCmd;
                    newCmd.m_Args = std::move(args);
                    strings.push_back(std::move(newCmd));
                    args.clear();
                }
                continue;
            } else {
                // Start of argument
                argStart = i;
            }
        }

        if (!isInQuotes) {
            if (c == ' ' || c == '\0' || c == ';' || c == '\n') {
                // End of argument
                size_t len = i - argStart;
                if (len != 0) {
                    args.push_back(std::string(cmd.substr(argStart, len)));
                }
                argStart = NO_ARG;

                if (c == ';' || c == '\n') {
                    // End of command
                    if (!args.empty()) {
                        CmdString newCmd;
                        newCmd.m_Args = std::move(args);
                        strings.push_back(std::move(newCmd));
                        args.clear();
                    }
                    continue;
                }
            } else if (c == '"') {
                // Start of quoted argument
                isInQuotes = true;
                argStart = i + 1;
            }
        } else {
            if (c == '"' || c == '\0') {
                // End of argument
                size_t len = i - argStart;
                if (len != 0) {
                    args.push_back(std::string(cmd.substr(argStart, len)));
                }
                argStart = NO_ARG;
                isInQuotes = false;
            }
        }
    }

    if (!args.empty()) {
        // Move what's left into a new command
        CmdString newCmd;
        newCmd.m_Args = std::move(args);
        strings.push_back(std::move(newCmd));
        args.clear();
    }

    return strings;
}

std::string appfw::CmdString::toString(const std::vector<CmdString> &arr) {
    std::string s;

    for (auto &i : arr) {
        s.append(i.toString());
        s.push_back(';');
        s.push_back(' ');
    }

    return s;
}
