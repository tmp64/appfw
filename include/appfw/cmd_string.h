#ifndef APPFW_CMD_STRING_H
#define APPFW_CMD_STRING_H
#include <vector>
#include <string>
#include <string_view>

namespace appfw {

class CmdString {
public:
    inline auto size() const { return m_Args.size(); }
    inline auto empty() const { return m_Args.empty(); }
    inline auto begin() const { return m_Args.begin(); }
    inline auto begin() { return m_Args.begin(); }
    inline auto end() const { return m_Args.end(); }
    inline auto end() { return m_Args.end(); }
    inline auto &operator[](size_t idx) const { return m_Args[idx]; }
    inline auto &operator[](size_t idx) { return m_Args[idx]; }
    inline const std::vector<std::string> &getArgs() const { return m_Args; }

    static std::vector<CmdString> parse(std::string_view str);

private:
    std::vector<std::string> m_Args;
};

} // namespace appfw

#endif
