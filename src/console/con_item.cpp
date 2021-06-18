#include <appfw/console/con_item.h>
#include <appfw/appfw.h>
#include <appfw/init.h>

//----------------------------------------------------------------
// ConItemBase
//----------------------------------------------------------------
appfw::ConItemBase::ConItemBase(std::string_view name, std::string_view descr) {
    AFW_ASSERT_MSG(!name.empty(), "ConItem name can't be empty");
    m_pName = name;
    m_pDescr = descr;

    if (appfw::isInitialized()) {
        // Constructed after appfw::initialize()
        // Register immediately
        getConsole().registerConItem(this);
    } else {
        // Constructed before appfw::initialize()
        // Push into the list of pending items
        getPendingItems().insert(this);
    }
}

appfw::ConItemBase::~ConItemBase() {
    if (appfw::isInitialized()) {
        // Destroyed before appfw::shutdown()
        getConsole().unregisterConItem(this);
    } else {
        // Destroyed after appfw::shutdown() or before appfw::initialize()
        // Remove from pending list
        if (!getPendingItems().empty()) {
            getPendingItems().erase(this);
        }
    }
}

std::set<appfw::ConItemBase *> &appfw::ConItemBase::getPendingItems() {
    static std::set<appfw::ConItemBase *> list;
    return list;
}

//----------------------------------------------------------------
// ConVarBase
//----------------------------------------------------------------
bool appfw::ConVarBase::isLocked() { return m_bIsLocked; }

void appfw::ConVarBase::setLocked(bool state) { m_bIsLocked = state; }

appfw::ConItemType appfw::ConVarBase::getType() { return ConItemType::ConVar; }

//----------------------------------------------------------------
// ConVar
//----------------------------------------------------------------
template <typename T>
appfw::ConVar<T>::ConVar(std::string_view name, const T &defValue, std::string_view descr,
                         Callback cb)
    : ConVarBase(name, descr) {
    setValue(defValue);
    setCallback(cb);
}

template <typename T>
const T &appfw::ConVar<T>::getValue() {
    return m_Value;
}

template <typename T>
appfw::VarSetResult appfw::ConVar<T>::setValue(const T &newVal) {
    if (isLocked()) {
        return VarSetResult::Locked;
    }

    if (m_Callback) {
        if (!m_Callback(m_Value, newVal)) {
            return VarSetResult::CallbackRejected;
        }
    }

    m_Value = newVal;
    return VarSetResult::Success;
}

template <typename T>
void appfw::ConVar<T>::setCallback(const Callback &callback) {
    m_Callback = callback;
}

template <typename T>
inline std::string appfw::ConVar<T>::getStringValue() {
    return convertValToString(m_Value);
}

template <typename T>
appfw::VarSetResult appfw::ConVar<T>::setStringValue(const std::string &val) {
    T newVal;
    if (!convertStringToVal(val, newVal)) {
        return VarSetResult::InvalidString;
    }

    return setValue(newVal);
}

template <typename T>
const char *appfw::ConVar<T>::getVarTypeAsString() {
    return typeNameToString<T>();
}

template class appfw::ConVar<bool>;
template class appfw::ConVar<int>;
template class appfw::ConVar<float>;
template class appfw::ConVar<double>;
template class appfw::ConVar<std::string>;

//----------------------------------------------------------------
// ConCommand
//----------------------------------------------------------------
appfw::ConCommand::ConCommand(std::string_view name, std::string_view descr)
    : ConItemBase(name, descr) {}

appfw::ConCommand::ConCommand(std::string_view name, std::string_view descr,
                              const Callback &callback)
    : ConCommand(name, descr) {
    setCallback(callback);
}

appfw::ConCommand::ConCommand(std::string_view name, std::string_view descr,
                              const CallbackNoArgs &callback)
    : ConCommand(name, descr) {
    setCallback(callback);
}

void appfw::ConCommand::execute(const CmdString &args) {
    if (!m_Callback) {
        printe("Command {} has no handler", getName());
    }
    m_Callback(args);
}

void appfw::ConCommand::setCallback(const Callback &callback) {
    m_Callback = callback;
}

void appfw::ConCommand::setCallback(const CallbackNoArgs &callback) {
    m_Callback = [callback](const CmdString &) { callback(); };
}

appfw::ConItemType appfw::ConCommand::getType() {
    return ConItemType::ConCommand;
}
