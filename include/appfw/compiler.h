#ifndef APPFW_COMPILER_H
#define APPFW_COMPILER_H
#include <appfw/platform.h>

#ifdef DOXYGEN

/**
 * Hit a breakpoint at the point of this macro.
 */
#define AFW_DEBUG_BREAK /* implementation defined */

/**
 * Force the compiler to inline the function.
 * Does not always work (depends on compiler).
 */
#define AFW_FORCE_INLINE /* implementation defined */

/**
 * Exports the symbol from this library.
 */
#define AFW_DLL_EXPORT /* implementation defined */

#elif COMPILER_MSVC

#include <intrin.h>

#define AFW_DEBUG_BREAK() __debugbreak()
#define AFW_FORCE_INLINE __forceinline
#define AFW_DLL_EXPORT _declspec(dllexport)

#elif COMPILER_GNU

#define AFW_DEBUG_BREAK() /* nope */
#define AFW_FORCE_INLINE __attribute__((always_inline))
#define AFW_DLL_EXPORT __attribute__((visibility("default")))

#endif

#endif
