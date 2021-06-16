#include <cassert>
#include <appfw/utils.h>
#include <appfw/init.h>

namespace {

struct AppfwLibrary {
    unsigned uInitCount = 0;
};

AppfwLibrary s_Lib;

}

//--------------------------------------------------------------
// Library initialization
//--------------------------------------------------------------
appfw::InitOptions &appfw::InitOptions::setArgs(int argc, char **argv) {
    iArgc = argc;
    ppszArgv = argv;
    return *this;
}

bool appfw::isInitialized() {
    return s_Lib.uInitCount > 0;
}

void appfw::initialize(const InitOptions &options) {
    if (s_Lib.uInitCount == 0) {
        s_Lib.uInitCount++;

        // TODO: Read options
        (void)options;
    }
}

void appfw::shutdown() {
    if (s_Lib.uInitCount == 0) {
        // Shutdown called too many times
        abort();
    }

    if (s_Lib.uInitCount == 1) {
        // TODO: Free resources
        s_Lib.uInitCount--;
    }
}
