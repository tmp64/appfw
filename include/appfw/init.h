#ifndef APPFW_INIT_H
#define APPFW_INIT_H
#include <appfw/utils.h>

namespace appfw {

struct InitOptions {
    int iArgc = 0;
    char **ppszArgv = nullptr;

    /**
     * Sets command line arguments to be parsed during init.
     */
    InitOptions &setArgs(int argc, char **argv);
};

/**
 * Returns whether appfw is initialized.
 */
bool isInitialized();

/**
 * Initializes the library.
 * This function can be called multiple times.
 */
void initialize(const InitOptions &options);

/**
 * De-initializes the library.
 * Must be called the same number of times as initialize() or
 * memory will be leaked and will trigger leak sanitizer.
 */
void shutdown();

struct InitComponent : NoMove {
    inline InitComponent(const InitOptions &options) { initialize(options); }
    inline ~InitComponent() { shutdown(); }
};

} // namespace appfw

#endif
