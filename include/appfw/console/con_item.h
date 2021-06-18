#ifndef APPFW_CONSOLE_CON_ITEM_H
#define APPFW_CONSOLE_CON_ITEM_H
#include <set>
#include <functional>
#include <string>
#include <string_view>

#include <appfw/cmd_string.h>
#include <appfw/utils.h>

namespace appfw {

class ConsoleSystem;

/**
 * An enumeration of all console item types.
 */
enum class ConItemType {
    None,
    ConVar,
    ConCommand,
    Any
};

enum class VarSetResult {
    Success = 0,
    InvalidString,
    CallbackRejected,
    Locked,
};

//----------------------------------------------------------------

/**
 * A base class for all console items.
 * Has a name and description (as const char *) and a specific type. 
 */
class ConItemBase : NoMove {
public:
    /**
     * Construct a console item.
     * @param name    Name of the item.
     * @param descr   Description of the item (can be nullptr).
     */
    ConItemBase(std::string_view name, std::string_view descr);
    virtual ~ConItemBase();

    /**
     * Returns name of this console item.
     */
    inline std::string_view getName() { return m_pName; }

    /**
     * Returns description of this console item.
     */
    inline std::string_view getDescr() { return m_pDescr; }

    /**
     * Returns type of the console item.
     */
    virtual ConItemType getType() = 0;

private:
    std::string_view m_pName;
    std::string_view m_pDescr;

    /**
     * Returns a reference to a list of unregistered items.
     */
    static std::set<ConItemBase *> &getPendingItems();

    friend class ConsoleSystem;
};

//----------------------------------------------------------------

/**
 * A base (typeless) class for console variables.
 */
class ConVarBase : public ConItemBase {
public:
    using ConItemBase::ConItemBase;

    /**
     * Returns whether cvar is locked or not.
     * When cvar is locked, it's value can't be changed.
     */
    bool isLocked();

    /**
     * Locks or unlocks the cvar.
     */
    void setLocked(bool state);

    /**
     * Returns string value of the convar.
     */
    virtual std::string getStringValue() = 0;

    /**
     * Sets a string value.
     * @return Success or failure (invalid string or callback rejected).
     */
    virtual VarSetResult setStringValue(const std::string &val) = 0;

    /**
     * Returns type of the cvar as a string (e.g. float, int).
     */
    virtual const char *getVarTypeAsString() = 0;

    virtual ConItemType getType() override;

private:
    bool m_bIsLocked = false;
};

//----------------------------------------------------------------

template <typename T>
class ConVar : public ConVarBase {
public:
    using Callback = std::function<bool(const T &oldVal, const T &newVal)>;

    ConVar(std::string_view name, const T &defValue, std::string_view descr,
           Callback cb = Callback());

    /**
     * Returns value of the convar.
     */
    const T &getValue();

    /**
     * Sets a new value of the cvar.
     * @return Whether or nos new value was applied.
     */
    VarSetResult setValue(const T &newVal);

    /**
     * Sets the value changed callback.
     */
    void setCallback(const Callback &callback);

    virtual std::string getStringValue() override;
    virtual VarSetResult setStringValue(const std::string &val) override;
    virtual const char *getVarTypeAsString() override;

private:
    T m_Value;
    Callback m_Callback;
};

//----------------------------------------------------------------

class ConCommand : public ConItemBase {
public:
    using Callback = std::function<void(const CmdString &args)>;
    using CallbackNoArgs = std::function<void()>;

    ConCommand(std::string_view name, std::string_view descr);
    ConCommand(std::string_view name, std::string_view descr, const Callback &callback);
    ConCommand(std::string_view name, std::string_view descr, const CallbackNoArgs &callback);

    /**
     * Runs the command.
     */
    void execute(const CmdString &args);

    /**
     * Sets the execute callback.
     */
    void setCallback(const Callback &callback);

    /**
     * Sets the execute callback.
     */
    void setCallback(const CallbackNoArgs &callback);

    virtual ConItemType getType() override;

private:
    Callback m_Callback;
};

} // namespace appfw

#endif
