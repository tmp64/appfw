#ifndef APPFW_CONSOLE_TERM_CONSOLE_H
#define APPFW_CONSOLE_TERM_CONSOLE_H
#include <appfw/console/console_system.h>

namespace appfw {

/**
 * A list of available terminal input methods
 */
enum class TermInputMethod
{
    Disable = 0, //!< Input by terminal is disabled, stdin is free to be used by the app
    Ignore, //!< stdin is grabbed, all input is ignored (if not supported, uses Disable instead)
    Enable, //!< stdin is grabbed, input is executed as commands
};

/**
 * Interface for a terminal console.
 * Terminal console is a console receiver that uses stdin and stdout.
 * There can only be one at the same time.
 */
class ITermConsole : public IConsoleReceiver {
public:
    /**
     * Returns current input method.
     */
    virtual TermInputMethod getInputMethod() = 0;

    /**
     * Called every main loop tick just before ConsoleSystem::processCommand().
     */
    virtual void tick() = 0;
};

} // namespace appfw

#endif
