#ifndef APPFW_STR_UTILS_H
#define APPFW_STR_UTILS_H
#include <cstring>

namespace appfw {

//! Comapres two ASCII strings ignoring the case.
//! @returns 0 if equal, < 0 if lexicographically less, > 0 if greater
inline int strcasecmp(const char *string1, const char *string2) {
#if PLATFORM_WINDOWS
    return ::_stricmp(string1, string2);
#else
    return ::strcasecmp(string1, string2);
#endif
}

//! Comapres two ASCII strings ignoring the case.
//! @returns 0 if equal, < 0 if lexicographically less, > 0 if greater
inline int strncasecmp(const char *string1, const char *string2, size_t n) {
#if PLATFORM_WINDOWS
    return ::_strnicmp(string1, string2, n);
#else
    return ::strncasecmp(string1, string2, n);
#endif
}

//! @returns whether the char is an ASCII whitespace char.
inline bool isAsciiSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

//! Converts an ASCII char to upper-case or leaves it as is.
inline char charToUpper(char c) {
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

//! Converts an ASCII char to lower-case or leaves it as is.
inline char charToLower(char c) {
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

//! Converts a null-terminated string's ASCII chars to upper-case.
inline void strToUpper(char *str) {
    while (*str != '\0') {
        *str = charToUpper(*str);
        str++;
    }
}

//! Converts a string's ASCII chars to upper-case.
template <typename Iter>
inline void strToUpper(Iter begin, Iter end) {
    while (begin != end) {
        *begin = charToUpper(*begin);
        begin++;
    }
}

//! Converts a null-terminated string's ASCII chars to upper-case.
inline void strToLower(char *str) {
    while (*str != '\0') {
        *str = charToLower(*str);
        str++;
    }
}

//! Converts a string's ASCII chars to upper-case.
template <typename Iter>
inline void strToLower(Iter begin, Iter end) {
    while (begin != end) {
        *begin = charToLower(*begin);
        begin++;
    }
}

//! Converts a string's ASCII chars to upper-case.
template <typename ConstIter, typename Iter>
inline void strToLower(ConstIter begin, ConstIter end, Iter out) {
    while (begin != end) {
        *out = charToLower(*begin);
        begin++;
        out++;
    }
}

} // namespace appfw

#endif
