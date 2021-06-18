#ifndef APPFW_DBG_H
#define APPFW_DBG_H
#include <cassert>
#include <string_view>
#include <functional>
#include <appfw/compiler.h>

namespace appfw {

enum class AssertAction
{
	Abort = 0, //!< Call abort()
	Ignore,    //!< Ignore the assertion
	Break,     //!< Break into the debugger if possible (ignore otherwise)
};

using AssertCallback = std::function<AssertAction(std::string_view cond, std::string_view msg,
                                                  std::string_view file, int line)>;

/**
 * Returns whether a debugger is attached at the moment.
 */
bool isDebuggerAttached();

/**
 * Called if an assertion fails.
 * @return whether to break into the debugger
 */
bool onAssertionFailed(std::string_view cond, std::string_view msg, std::string_view file,
                               int line);

/**
 * Sets the callback that will be called on assertion failure.
 */
void setAssertCallback(const AssertCallback &cb);

/**
 * Checks whether the condition is true. If not, crashes the program.
 * The check will be performed in both debug and relese builds.
 * @param   cond    The condition to check
 * @param   msg     Message if it fails
 */
#define AFW_ASSERT_REL_MSG(cond, msg)                                                              \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            if (appfw::onAssertionFailed(#cond, (msg), __FILE__, __LINE__)) {                      \
                AFW_DEBUG_BREAK();                                                                 \
            }                                                                                      \
        }                                                                                          \
    } while (false)

/**
 * Checks whether the condition is true. If not, crashes the program.
 * The check will be performed in both debug and relese builds.
 * @param   cond    The condition to check
 */
#define AFW_ASSERT_REL(cond) AFW_ASSERT_REL_MSG(cond, "")

#if AFW_DEBUG_BUILD

/**
 * Checks whether the condition is true. If not, crashes the program.
 * The check will be performed only in debug builds.
 * @param   cond    The condition to check
 * @param   msg     Message if it fails
 */
#define AFW_ASSERT_MSG(cond, msg) AFW_ASSERT_REL_MSG(cond, msg)

/**
 * Checks whether the condition is true. If not, crashes the program.
 * The check will be performed only in debug builds.
 * @param   cond    The condition to check
 */
#define AFW_ASSERT(cond) AFW_ASSERT_REL(cond)

#else

/**
 * Assertions are disabled in debug builds.
 * Use AFW_ASSERT_REL_MSG instead.
 */
#define AFW_ASSERT_MSG(cond, msg) /* disabled */

/**
 * Assertions are disabled in debug builds.
 * Use AFW_ASSERT_REL instead.
 */
#define AFW_ASSERT(cond) /* disabled */

#endif // AFW_DEBUG_BUILD

} // namespace appfw

#endif
