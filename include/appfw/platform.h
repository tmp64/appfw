#ifndef APPFW_PLATFORM_H
#define APPFW_PLATFORM_H

#include <cstdint>

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_GNU

#endif

namespace appfw {

//----------------------------------------------------------------

#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
#define AFW_DEBUG_BUILD 1
#else
#define AFW_DEBUG_BUILD 0
#endif

/**
 * Returns true if this is a debug build.
 */
constexpr bool isDebugBuild() { return AFW_DEBUG_BUILD; }

//----------------------------------------------------------------

/**
 * Returns true when building for Windows.
 */
constexpr bool isWindows() { return PLATFORM_WINDOWS; }

/**
 * Returns true when building for a UNIX-like OS.
 */
constexpr bool isUnix() { return PLATFORM_UNIX; }

/**
 * Returns true when building for Linux.
 */
constexpr bool isLinux() { return PLATFORM_LINUX; }

/**
 * Returns true when building for Android.
 */
constexpr bool isAndroid() { return PLATFORM_ANDROID; }

/**
 * Returns true when building for macOS.
 */
constexpr bool isMacOS() { return PLATFORM_MACOS; }

//----------------------------------------------------------------

/**
 * Returns true when compiling with Visual C++ compiler.
 */
constexpr bool isMSVC() { return COMPILER_MSVC; }

/**
 * Returns true when compiling with GCC-compatible compiler.
 */
constexpr bool isGNU() { return COMPILER_GNU; }

/**
 * Returns true when compiling with GCC.
 */
constexpr bool isGCC() { return COMPILER_GCC; }

/**
 * Returns true when compiling with Clang.
 */
constexpr bool isClang() { return COMPILER_CLANG; }

//----------------------------------------------------------------

/**
 * Returns the name of the OS app is built for.
 */
constexpr const char *getOSName() {
    static_assert(isWindows() || isUnix(), "Unknown target OS");

    if (isWindows()) {
        return "Windows";
    } else if (isUnix()) {
        if (isLinux()) {
            if (isAndroid()) {
                return "Android";
            } else {
                return "Linux";
            }
        } else if (isMacOS()) {
            return "macOS";
        } else {
            return "Unix-like";
        }
    }

    return "Unknown OS";
}

/**
 * Returns the name of the compiler app is built with.
 */
constexpr const char *getCompilerName() {
    static_assert(isMSVC() || isGNU(), "Unknown compiler");

    if (isMSVC()) {
        return "Visual C++";
    } else if (isGNU()) {
        if (isGCC()) {
            return "GCC";
        }

        if (isClang()) {
            return "Clang";
        }

        return "GNU-compatible";
    }

    return "Unknown compiler";
}

/**
 * Returns date and time when this translation unit was built.
 * Format: Month DD YYYY HH:MM:SS
 * e.g. Jul 19 2020 19:41:12
 */
constexpr const char *getBuildDate() { return __DATE__ " " __TIME__; }

//----------------------------------------------------------------

/**
 * Returns true if building for Big-Endian platform.
 */
constexpr bool isBigEndian() { return PLATFORM_BIG_ENDIAN; }

/**
 * Returns true if building for Little-Endian platform.
 */
constexpr bool isLittleEndian() { return PLATFORM_LITTLE_ENDIAN; }

static_assert(isBigEndian() != isLittleEndian(), "Platform must be either big or little endian");

//----------------------------------------------------------------

template <typename T>
inline T swapByteOrder(T input) {
	// FIXME:
    //static_assert(appfw::utils::FalseT<T>::value, "T bytes cannot be swapped");
}

template <>
inline uint16_t swapByteOrder(uint16_t input) {
#if COMPILER_MSVC
    return _byteswap_ushort(input);
#elif COMPILER_GNU
    return __builtin_bswap16(input);
#else
    return ((input & 0xFF) << 8) | ((input & 0xFF00) >> 8);
#endif
}

template <>
inline uint32_t swapByteOrder(uint32_t input) {
#if COMPILER_MSVC
    return _byteswap_ulong(input);
#elif COMPILER_GNU
    return __builtin_bswap32(input);
#else
    return
        ((input & 0x000000FFu) << 24) |
        ((input & 0x0000FF00u) << 8) |
        ((input & 0x00FF0000u) >> 8) |
        ((input & 0xFF000000u) >> 24);
#endif
}

template <>
inline uint64_t swapByteOrder(uint64_t input) {
#if COMPILER_MSVC
    return _byteswap_uint64(input);
#elif COMPILER_GNU
    return __builtin_bswap64(input);
#else
    return
        ((input & 0x00000000000000FFul) << 56) |
        ((input & 0x000000000000FF00ul) << 40) |
        ((input & 0x0000000000FF0000ul) << 24) |
        ((input & 0x00000000FF000000ul) << 8) |
        ((input & 0x000000FF00000000ul) >> 8) |
        ((input & 0x0000FF0000000000ul) >> 24) |
        ((input & 0x00FF000000000000ul) >> 40) |
        ((input & 0xFF00000000000000ul) >> 56);
#endif
}

template <>
inline int16_t swapByteOrder(int16_t input) {
    return (int16_t)swapByteOrder((uint16_t)input);
}

template <>
inline int32_t swapByteOrder(int32_t input) {
    return (int32_t)swapByteOrder((uint32_t)input);
}

template <>
inline int64_t swapByteOrder(int64_t input) {
    return (int64_t)swapByteOrder((uint64_t)input);
}

/**
 * Converts between platform byte order and little-endian.
 */
template <typename T>
inline T littleEndianSwap(T input) {
    if constexpr (isLittleEndian()) {
        return input;
    } else {
        return swapByteOrder(input);
    }
}

/**
 * Converts between platform byte order and big-endian.
 */
template <typename T>
inline T bigEndianSwap(T input) {
    if constexpr (isBigEndian()) {
        return input;
    } else {
        return swapByteOrder(input);
    }
}

} // namespace appfw

#endif
