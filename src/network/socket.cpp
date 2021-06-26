#include <fmt/format.h>
#include <appfw/network/socket.h>
#include "plat_sockets.h"

//----------------------------------------------------------------
// SockFd
//----------------------------------------------------------------
appfw::SockFd::SockFd(SockFd &&other) noexcept {
    m_ifd = other.m_ifd;
    other.m_ifd = 0;
}

appfw::SockFd &appfw::SockFd::operator=(SockFd &&other) noexcept {
    if (&other == this) {
        return *this;
    }

    close();
    m_ifd = other.m_ifd;
    other.m_ifd = 0;
    return *this;
}

void appfw::SockFd::set(SocketFile fd) {
    close();
    AFW_ASSERT(fd >= 0);
    m_ifd = fd;
}

void appfw::SockFd::close() {
    AFW_ASSERT(m_ifd >= 0);

    if (m_ifd != 0) {
#if PLATFORM_WINDOWS
        ::closesocket(m_ifd);
#elif PLATFORM_UNIX
        ::close(m_ifd);
#else
#error
#endif
        m_ifd = 0;
    }
}

//----------------------------------------------------------------
// SocketErrorException
//----------------------------------------------------------------
namespace {

#if PLATFORM_WINDOWS
std::string getWSAErrorText(int error) {
    wchar_t *wbuf = nullptr;

    try {
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                       0, error, 0, (LPWSTR)&wbuf, 1, nullptr);

        if (!wbuf) {
            return "< error message alloc error >";
        }

        int bufSize = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
        std::vector<char> buf(bufSize + 1);
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf.data(), bufSize + 1, nullptr, nullptr);
        LocalFree(wbuf);
        wbuf = nullptr;
        return std::string(buf.data());
    }
    catch (...) {
        if (wbuf) {
            LocalFree(wbuf);
            wbuf = nullptr;
        }

        return "< error message exception >";
    }
}
#elif PLATFORM_UNIX
std::string getErrnoText(int error) {
    return strerror(error);
}
#endif

}

appfw::NetworkErrorException::NetworkErrorException(std::string_view msg)
    : std::runtime_error(std::string(msg)) {}

appfw::ConnectionTimedOutException::ConnectionTimedOutException()
    : ConnectionTimedOutException("connection timed-out") {}

appfw::ConnectionTimedOutException::ConnectionTimedOutException(std::string_view msg)
    : NetworkErrorException(msg) {}

#if PLATFORM_WINDOWS
appfw::SocketErrorException::SocketErrorException(std::string_view msg)
    : SocketErrorException(msg, WSAGetLastError()) {}

appfw::SocketErrorException::SocketErrorException(std::string_view msg, int error)
    : NetworkErrorException(fmt::format("{}: {}", msg, getWSAErrorText(error))) {
    error = m_iError;
}
#elif PLATFORM_UNIX
appfw::SocketErrorException::SocketErrorException(std::string_view msg)
    : SocketErrorException(msg, errno) {}

appfw::SocketErrorException::SocketErrorException(std::string_view msg, int error)
    : NetworkErrorException(fmt::format("{}: {} (errno {})", msg, getErrnoText(error), error)) {
    error = m_iError;
}
#else
#error
#endif
