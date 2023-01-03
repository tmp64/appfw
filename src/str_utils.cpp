#include <appfw/str_utils.h>

int appfw::strReplace(std::string &str, std::string_view from, std::string_view to) {
    int count = 0;
    size_t pos = str.find(from);

    while (pos != str.npos) {
        str.replace(pos, from.size(), to);
        pos = str.find(from, pos + to.size());
    }

    return count;
}
