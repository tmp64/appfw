#ifndef APPFW_NETWORK_SOCKET_H
#define APPFW_NETWORK_SOCKET_H
#include <stdexcept>
#include <string_view>
#include <appfw/network/sock_addr.h>
#include <appfw/dbg.h>

namespace appfw {

#if PLATFORM_WINDOWS
using SocketFile = uintptr_t;
constexpr SocketFile NULL_SOCKET = (SocketFile)(~(0));
#elif PLATFORM_UNIX
using SocketFile = int;
constexpr SocketFile NULL_SOCKET = -1;
#else
#error
#endif

enum class SocketCloseReason
{
    Failure,     //!< General failure
    ConnAborted, //!< Closed by the other side
    TimeOut,     //!< Timed out
    User,        //!< Closed by user
    Shutdown,    //!< Server is shutting down
};

enum class NetClientStatus
{
    Closed,     //!< Not connected
    Connecting, //!< Connecting
    Connected,  //!< Connection established
};

/**
 * RAII wrapper for sockets.
 */
class SockFd {
public:
    SockFd() = default;
    inline explicit SockFd(SocketFile fd) { m_ifd = fd; }
    inline ~SockFd() { close(); }

    SockFd(const SockFd &) = delete;
    SockFd &operator=(const SockFd &) = delete;

    SockFd(SockFd &&other) noexcept;
    SockFd &operator=(SockFd &&other) noexcept;

    inline SocketFile get() { return m_ifd; }
    void set(SocketFile fd);
    void close();

private:
    SocketFile m_ifd = 0;
};

/**
 * An exception class for general network errors.
 */
class NetworkErrorException : public std::runtime_error {
public:
    explicit NetworkErrorException(std::string_view msg);
};

/**
 * An exception class for a time-out error.
 */
class ConnectionTimedOutException : public NetworkErrorException {
public:
    ConnectionTimedOutException();
    explicit ConnectionTimedOutException(std::string_view msg);
};

/**
 * An exception class for socket errors.
 * what() will containt msg as well as a system error (errno or WSAGetLastError) message
 */
class SocketErrorException : public NetworkErrorException {
public:
    explicit SocketErrorException(std::string_view msg);
    SocketErrorException(std::string_view msg, int error);

    inline int getError() const { return m_iError; }

private:
    int m_iError = 0;
};

} // namespace appfw 

#endif
