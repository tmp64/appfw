#include <cassert>
#include <appfw/console/std_console.h>
#include <appfw/appfw.h>
#include <appfw/utils.h>
#include <appfw/init.h>

namespace {

struct AppfwLibrary {
    unsigned uInitCount = 0;
    appfw::CommandLine cmdLine;
    appfw::FileSystem fileSystem;
    appfw::manual_ptr<appfw::ConsoleSystem> pConSys;
    appfw::manual_ptr<appfw::ITermConsole> pTermConsole;
};

AppfwLibrary s_Lib;

} // namespace

//--------------------------------------------------------------
// Getters
//--------------------------------------------------------------
appfw::ConsoleSystem &appfw::getConsole() {
    return *s_Lib.pConSys;
}

appfw::CommandLine &appfw::getCommandLine() {
    return s_Lib.cmdLine;
}

appfw::FileSystem &appfw::getFileSystem() {
    return s_Lib.fileSystem;
}

//--------------------------------------------------------------
// Library initialization
//--------------------------------------------------------------
appfw::InitOptions &appfw::InitOptions::setArgs(int argc, char **argv) {
    iArgc = argc;
    ppszArgv = argv;
    return *this;
}

appfw::InitOptions &appfw::InitOptions::setInputMethod(TermInputMethod method) {
    inputMethod = method;
    return *this;
}

bool appfw::isInitialized() {
    return s_Lib.uInitCount > 0;
}

void appfw::initialize(const InitOptions &options) {
    if (s_Lib.uInitCount == 0) {
        // Init console system
        s_Lib.pConSys = new ConsoleSystem();

        // Add terminal console receiver
        s_Lib.pTermConsole = new StdConsole(options.inputMethod);
        s_Lib.pConSys->addConsoleReceiver(s_Lib.pTermConsole.get());

        // Parse arguments
        if (options.iArgc > 0) {
            AFW_ASSERT(options.ppszArgv);
            getCommandLine().parseCommandLine(options.iArgc, options.ppszArgv, true);
        }
    }

    s_Lib.uInitCount++;
}

void appfw::shutdown() {
    if (s_Lib.uInitCount == 0) {
        // Shutdown called too many times
        abort();
    }

    s_Lib.uInitCount--;

    if (s_Lib.uInitCount == 0) {
        // Shutdown console receiver
        if (s_Lib.pTermConsole) {
            s_Lib.pConSys->removeConsoleReceiver(s_Lib.pTermConsole.get());
            s_Lib.pTermConsole.reset();
        }

        // Shutdown console system
        s_Lib.pConSys.reset();
    }
}

void appfw::mainLoopTick() {
    if (s_Lib.pTermConsole) {
        s_Lib.pTermConsole->tick();
    }

    s_Lib.pConSys->processCommand();
    s_Lib.pConSys->processMsgQueue();
}
