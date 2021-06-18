#include <appfw/appfw.h>
#include <appfw/dbg.h>
#include <appfw/init.h>

#if PLATFORM_WINDOWS
#include <appfw/windows.h>
#endif

namespace {

appfw::AssertCallback s_AssertCb;

appfw::AssertAction defaultAssertCallback(std::string_view, std::string_view, std::string_view, int) {
    if (appfw::isDebuggerAttached()) {
        return appfw::AssertAction::Break;
    } else {
        return appfw::AssertAction::Abort;
    }
}

}

bool appfw::isDebuggerAttached() {
#if PLATFORM_WINDOWS
    return IsDebuggerPresent();
#else
    return false;
#endif
}

bool appfw::onAssertionFailed(std::string_view cond, std::string_view msg, std::string_view file,
                              int line) {
    if (appfw::isInitialized()) {
        printwtf("Assertion failed: {}", cond);
        if (!msg.empty()) {
            printwtf("Reason: {}", msg);
        }
        printwtf("File: {}:{}", file, line);
    }

    AssertAction action;

    if (s_AssertCb) {
        action = s_AssertCb(cond, msg, file, line);
    } else {
        action = defaultAssertCallback(cond, msg, file, line);
    }

    switch (action) {
    case AssertAction::Abort: {
        abort();
        return false;
    }
    case AssertAction::Ignore: {
        return false;
    }
    case AssertAction::Break: {
        return true;
    }
    default: {
        // assertception!
        // AFW_ASSERT(false);
        abort();
        return false;
    }
    }
}

void appfw::setAssertCallback(const AssertCallback &cb) {
    s_AssertCb = cb;
}
