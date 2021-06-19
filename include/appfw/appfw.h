#ifndef APPFW_APPFW_H
#define APPFW_APPFW_H
#include <fmt/format.h>
#include <appfw/console/console_system.h>
#include <appfw/command_line.h>
#include <appfw/compiler.h>
#include <appfw/dbg.h>
#include <appfw/platform.h>
#include <appfw/utils.h>
#include "this_module_info.h"

namespace appfw {

/**
 * Contains properties of current module.
 */
struct ModuleInfo {
    const char *pszModuleName;
};

ConsoleSystem &getConsole();
CommandLine &getCommandLine();

} // namespace appfw

//----------------------------------------------------------------
// Using declarations
//----------------------------------------------------------------
using appfw::ConVar;
using appfw::ConCommand;
using appfw::ConMsgType;
using appfw::ConMsgColor;

namespace detail {
namespace MODULE_NAMESPACE {

//----------------------------------------------------------------
// Module info
//----------------------------------------------------------------
inline const appfw::ModuleInfo &getModuleInfo() {
    static appfw::ModuleInfo a{MODULE_NAME};
    return a;
}

//----------------------------------------------------------------
// Console output
//----------------------------------------------------------------
inline void vConPrint(appfw::ConMsgInfo info, std::string_view format, fmt::format_args args) {
    info.setTag(MODULE_NAME);
    appfw::getConsole().print(info, fmt::vformat(format, args));
}

// WTF
template <typename... Args>
inline void printwtf(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::WTF), format, fmt::make_format_args(args...));
}

template <typename... Args>
inline void printwtf(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::WTF).setColor(color), format,
              fmt::make_format_args(args...));
}

// Fatal
template <typename... Args>
inline void printfatal(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Fatal), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printfatal(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Fatal).setColor(color), format,
              fmt::make_format_args(args...));
}

// Error
template <typename... Args>
inline void printe(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Error), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printe(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Error).setColor(color), format,
              fmt::make_format_args(args...));
}

// Warning
template <typename... Args>
inline void printw(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Warn), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printw(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Warn).setColor(color), format,
              fmt::make_format_args(args...));
}

// Notice
template <typename... Args>
inline void printn(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Notice), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printn(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Notice).setColor(color), format,
              fmt::make_format_args(args...));
}

// Info
template <typename... Args>
inline void printi(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Info), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printi(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Info).setColor(color), format,
              fmt::make_format_args(args...));
}

// Debug
template <typename... Args>
inline void printd(std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Debug), format,
              fmt::make_format_args(args...));
}

template <typename... Args>
inline void printd(ConMsgColor color, std::string_view format, const Args &...args) {
    vConPrint(appfw::ConMsgInfo().setType(ConMsgType::Debug).setColor(color), format,
              fmt::make_format_args(args...));
}

} // namespace MODULE_NAMESPACE
} // namespace detail

using namespace detail::MODULE_NAMESPACE;

#endif
