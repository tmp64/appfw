#ifndef APPFW_CONSOLE_CON_MSG_H
#define APPFW_CONSOLE_CON_MSG_H
#include <array>
#include <ctime>
#include <cstdint>

namespace appfw {

/**
 * Describes different console message severity levels/types.
 */
enum class ConMsgType : uint8_t
{
    WTF = 0, //<! Something impossible happened (indicates a bug)
    Fatal,   //!< Fatal Error: most of the time unrecoverable
    Error,   //!< Error: something went wrong
    Warn,    //!< Warning: something unexpected (but not critical) happened
    Notice,  //!< Notice: something requires attention
    Info,    //!< Info: just an informational message
    Debug,   //!< Debug: diagnostic output
    Input,   //!< Input: special type for console input echo
};

/**
 * Describes a 4-bit RGB color of a message.
 * Format: 0bIRGB
 */
enum class ConMsgColor : uint8_t
{
    Default = 0, //!< Default color by message type
    Blue,
    Green,
    Cyan,
    Red,
    Purple,
    Yellow,
    Grey,
    Black,
    BrightBlue,
    BrightGreen,
    BrightCyan,
    BrightRed,
    BrightPurple,
    BrightYellow,
    White,
};

struct ConMsgInfo {
    //! Type of the message
    ConMsgType type = ConMsgType::Info;

    //! Color of the message
    ConMsgColor color = ConMsgColor::Default;

    //! Time the message was created
    time_t time = std::time(nullptr);

    //! Tag of the message source
    const char *tag = "< unknown >";

    inline ConMsgInfo &setType(ConMsgType t) { type = t; return *this; }
    inline ConMsgInfo &setColor(ConMsgColor c) { color = c; return *this; }
    inline ConMsgInfo &setTag(const char *t) { tag = t; return *this; }
};

using ConMsg = std::pair<ConMsgInfo, std::string>;

namespace detail::conmsg {

constexpr ConMsgColor typeColors[] = {
    ConMsgColor::BrightRed,
    ConMsgColor::BrightRed,
    ConMsgColor::BrightRed,
    ConMsgColor::BrightYellow,
    ConMsgColor::White,
    ConMsgColor::Grey,
    ConMsgColor::BrightBlue,
    ConMsgColor::White,
};

}

/**
 * Returns the default color of a message type
 */
constexpr ConMsgColor getMsgTypeColor(ConMsgType type) {
    return detail::conmsg::typeColors[(int)type];
}

} // namespace appfw

#endif
