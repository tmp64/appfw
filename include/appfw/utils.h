#ifndef APPFW_UTILS_H
#define APPFW_UTILS_H
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <future>
#include <appfw/dbg.h>

namespace appfw {

/**
 * A dummy templated struct to be used with static_assert and templates.
 * Use like this: static_assert(FalseT<T>::value, "Bad")
 */
template <typename T>
struct FalseT : std::false_type {};

//----------------------------------------------------------------

/**
 * An uncopyable class.
 * Inherit from it to make your class uncopyable as well.
 */
class NoCopy {
public:
    NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;

    NoCopy(NoCopy &&) = default;
    NoCopy &operator=(NoCopy &&) = default;
};

/**
 * An unmoveable class.
 * Inherit from it to make your class uncopyable as well.
 */
class NoMove {
public:
    NoMove() = default;

    NoMove(const NoMove &) = delete;
    NoMove &operator=(const NoMove &) = delete;

    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;
};

//----------------------------------------------------------------

/**
 * Converts a value of type T into a string.
 */
template <typename T>
inline std::string convertValToString(const T &val) {
    return std::to_string(val);
}

/**
 * Returns string unchanged.
 */
template <>
inline std::string convertValToString<std::string>(const std::string &val) {
    return val;
}

//----------------------------------------------------------------

/**
 * Converts contents of a string into a type T.
 * T can be one of boolean, integer (except long) and float types or a string.
 * @return true if converted successfully, false otherwise.
 */
template <typename T>
bool convertStringToVal(const std::string &str, T &val) {
    static_assert(FalseT<T>::value, "T cannot be made from string");
    return false;
}

extern template bool convertStringToVal<bool>(const std::string &str, bool &val);
extern template bool convertStringToVal<short>(const std::string &str, short &val);
extern template bool convertStringToVal<unsigned short>(const std::string &str,
                                                        unsigned short &val);
extern template bool convertStringToVal<int>(const std::string &str, int &val);
extern template bool convertStringToVal<unsigned int>(const std::string &str, unsigned int &val);
extern template bool convertStringToVal<long>(const std::string &str, long &val);
extern template bool convertStringToVal<unsigned long>(const std::string &str, unsigned long &val);
extern template bool convertStringToVal<long long>(const std::string &str, long long &val);
extern template bool convertStringToVal<unsigned long long>(const std::string &str,
                                                            unsigned long long &val);
extern template bool convertStringToVal<float>(const std::string &str, float &val);
extern template bool convertStringToVal<double>(const std::string &str, double &val);
extern template bool convertStringToVal<std::string>(const std::string &str, std::string &val);

//----------------------------------------------------------------

template <typename T>
constexpr const char *typeNameToString() {
    static_assert(FalseT<T>::value, "typeNameToString: template instantiation for T was not found");
    return nullptr;
}

template <>
constexpr const char *typeNameToString<int>() {
    return "int";
}

template <>
constexpr const char *typeNameToString<float>() {
    return "float";
}

template <>
constexpr const char *typeNameToString<double>() {
    return "float";
}

template <>
constexpr const char *typeNameToString<std::string>() {
    return "string";
}

template <>
constexpr const char *typeNameToString<bool>() {
    return "bool";
}

//----------------------------------------------------------------

/**
 * Seeks the file to the beginning and returns its size.
 */
inline int64_t getFileSize(std::ifstream &file) {
    file.seekg(0, file.end);
    int64_t size = file.tellg();
    file.seekg(0, file.beg);
    return size;
}

/**
 * Reads contents of a **binary** file into a char vector.
 */
inline void readFileContents(std::ifstream &file, std::vector<char> &data) {
    int64_t size64 = getFileSize(file);
    if (size64 > std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("file is too large");
    }
    size_t size = (size_t)size64;
    data.resize(size);
    file.read((char *)data.data(), size);
}

/**
 * Reads contents of a **binary** file into a byte vector.
 */
inline void readFileContents(std::ifstream &file, std::vector<uint8_t> &data) {
    int64_t size64 = getFileSize(file);
    if (size64 > std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("file is too large");
    }
    size_t size = (size_t)size64;
    data.resize(size);
    file.read((char *)data.data(), size);
}

/**
 * Returns whether a future is ready.
 */
template <typename R>
inline bool isFutureReady(const std::future<R> &f) {
    return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

//----------------------------------------------------------------

/**
 * A "smart" pointer that would abort() if it wasn't freed manually.
 */
template <typename T>
class manual_ptr {
public:
    manual_ptr() = default;
    manual_ptr(T *p) { m_pPtr = p; }
    ~manual_ptr() {
        if (m_pPtr) {
            AFW_ASSERT_MSG(false, "appfw::manual_ptr was not freed");
            std::abort();
        }
    }

    manual_ptr<T> &operator=(T *p) {
        m_pPtr = std::unique_ptr<T>(p);
        return *this;
    }

    T *get() { return m_pPtr.get(); }
    T &operator*() { return *m_pPtr; }
    T *operator->() { return m_pPtr.get(); }

    explicit operator bool() { return m_pPtr.get() != nullptr; }

    void reset() { m_pPtr.reset(); }

private:
    std::unique_ptr<T> m_pPtr = nullptr;
};

} // namespace appfw

#endif
