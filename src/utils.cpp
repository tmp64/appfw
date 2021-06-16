#include <fmt/format.h>
#include <appfw/utils.h>

//--------------------------------------------------------------
// Implementations of convertStringToVal
//--------------------------------------------------------------
namespace {

template <typename T>
using ConvFuncInt = T (*)(const std::string &str, size_t *idx, int base);

template <typename T>
using ConvFuncFloat = T (*)(const std::string &str, size_t *idx);

template <typename T>
inline bool convertStringToValDetailInt(ConvFuncInt<T> f, const std::string &str, T &val) {
    try {
        val = f(str, nullptr, 10);
        return true;
    } catch (...) {
        return false;
    }
}

template <typename T>
inline bool convertStringToValDetailFloat(ConvFuncFloat<T> f, const std::string &str, T &val) {
    try {
        val = f(str, nullptr);
        return true;
    } catch (...) {
        return false;
    }
}

}

// Long long
template <>
bool appfw::convertStringToVal(const std::string &str, long long &val) {
    return convertStringToValDetailInt(&std::stoll, str, val);
}

// Unsigned long long
template <>
bool appfw::convertStringToVal(const std::string &str, unsigned long long &val) {
    return convertStringToValDetailInt(&std::stoull, str, val);
}

// Long
template <>
bool appfw::convertStringToVal(const std::string &str, long &val) {
    return convertStringToValDetailInt(&std::stol, str, val);
}

// Unsigned long
template <>
bool appfw::convertStringToVal(const std::string &str, unsigned long &val) {
    return convertStringToValDetailInt(&std::stoul, str, val);
}

// Int
template<>
bool appfw::convertStringToVal(const std::string &str, int &val) {
    return convertStringToValDetailInt(&std::stoi, str, val);
}

// Unsigned int
template <>
bool appfw::convertStringToVal(const std::string &str, unsigned int &val) {
    unsigned long iVal = 0;

    if (!convertStringToVal(str, iVal)) {
        return false;
    }

    if (iVal < std::numeric_limits<unsigned int>::min() &&
        iVal > std::numeric_limits<unsigned int>::max()) {
        return false;
    }

    val = iVal;

    return true;
}

// Short
template <>
bool appfw::convertStringToVal(const std::string &str, short &val) {
    int iVal = 0;

    if (!convertStringToVal(str, iVal)) {
        return false;
    }

    if (iVal < std::numeric_limits<short>::min() && iVal > std::numeric_limits<short>::max()) {
        return false;
    }

    val = iVal;

    return true;
}

// Unsigned short
template <>
bool appfw::convertStringToVal(const std::string &str, unsigned short &val) {
    unsigned int iVal = 0;

    if (!convertStringToVal(str, iVal)) {
        return false;
    }

    if (iVal < std::numeric_limits<unsigned short>::min() &&
        iVal > std::numeric_limits<unsigned short>::max()) {
        return false;
    }

    val = iVal;

    return true;
}

// Bool
template <>
bool appfw::convertStringToVal(const std::string &str, bool &val) {
    int iVal = 0;
    if (!convertStringToVal(str, iVal)) {
        return false;
    }
    val = !!iVal;
    return true;
}

// Float
template <>
bool appfw::convertStringToVal(const std::string &str, float &val) {
    return convertStringToValDetailFloat(&std::stof, str, val);
}

// Double
template <>
bool appfw::convertStringToVal(const std::string &str, double &val) {
    return convertStringToValDetailFloat(&std::stod, str, val);
}

// String
template <>
bool appfw::convertStringToVal(const std::string &str, std::string &val) {
    val = str;
    return true;
}
