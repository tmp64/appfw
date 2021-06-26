#include <appfw/network/tcp_client4.h>
#include "plat_sockets.h"

void appfw::TcpClient4::connect(const SockAddr4 &addr, int timeout) {
    if (m_Status != NetClientStatus::Closed) {
        throw std::logic_error("already connected");
    }

    try {
        platsock::initNetworking();

        int result = 0;
        m_Addr = addr;
        sockaddr_in sockAddr = addr.toSockAddrStruct();

        // Open the socket
        {
            SocketFile sockfd = ::socket(PF_INET, SOCK_STREAM, 0);

            if (sockfd == NULL_SOCKET) {
                throw SocketErrorException("socket() failed");
            }

            m_fd.set(sockfd);
            m_Status = NetClientStatus::Connecting;
        }

        // Set non-blocking mode
        if (!platsock::setSocketBlockingMode(m_fd.get(), false)) {
            throw SocketErrorException("setSocketBlockingMode(false) failed");
        }

        // Connect
        sockaddr_in sa = addr.toSockAddrStruct();
        result = ::connect(m_fd.get(), reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
        if (result == 0) {
            m_Status = NetClientStatus::Connected;
        } else {
#if PLATFORM_WINDOWS
            int error = WSAGetLastError();

            if (error != WSAEWOULDBLOCK) {
                close();
                throw SocketErrorException("connect() failed", error);
            }
#elif PLATFORM_UNIX
            int error = errno;

            if (error != EINPROGRESS) {
                close();
                throw SocketErrorException("connect() failed", error);
            }
#else
#error
#endif
        }

        m_iTimeOut = timeout;
        m_Timer.start();
    } catch (...) {
        close();
        throw;
    }
}

bool appfw::TcpClient4::updateStatus(int time) {
    if (m_Status == NetClientStatus::Closed) {
        throw std::logic_error("not connecting or not connected");
    }

    AFW_ASSERT(m_fd.get() != 0);

    timeval tv;
    tv.tv_sec = time / 1000;
    tv.tv_usec = time % 1000;

    if (m_Status == NetClientStatus::Connecting) {
        fd_set writefds, exceptfds;

        FD_ZERO(&writefds);
        FD_SET(m_fd.get(), &writefds);

        FD_ZERO(&exceptfds);
        FD_SET(m_fd.get(), &exceptfds);

        int result =
            ::select(m_fd.get() + 1, nullptr, &writefds , & exceptfds, (time == -1) ? nullptr : &tv);

        if (result == 0) {
            // Still connecting
            return false;
        } else if (result == 1) {
            // Get the result
            int error = 0;
#if PLATFORM_WINDOWS
            int errorLen = sizeof(error);
#else
            socklen_t errorLen = sizeof(error);
#endif
            int getResult = ::getsockopt(m_fd.get(), SOL_SOCKET, SO_ERROR,
                                         reinterpret_cast<char *>(&error), &errorLen);

            if (getResult != 0) {
                auto ex = SocketErrorException("getsockopt() failed");
                close();
                throw ex;
            }

            if (error == 0) {
                // Connection established
                m_Status = NetClientStatus::Connected;
                return false;
            }

            [[maybe_unused]] bool isTimeOut = false;

#if PLATFORM_WINDOWS
            if (error != WSAEWOULDBLOCK) {
                close();
                throw SocketErrorException("connect() failed", error);
            }
#elif PLATFORM_UNIX
            if (error == ETIMEDOUT) {
                isTimeOut = true;
            } else if (error != EINPROGRESS) {
                close();
                throw SocketErrorException("connect() failed", error);
            }
#else
#error
#endif
            // Check time out
            if (isTimeOut || m_Timer.ms() >= m_iTimeOut) {
                // Connection timed out
                m_Timer.stop();
                close();
                throw ConnectionTimedOutException();
            }
        } else {
            // Error
            auto ex = SocketErrorException("select() failed");
            close();
            throw ex;
        }
    } else if (m_Status == NetClientStatus::Connected) {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(m_fd.get(), &readfds);

        int result =
            ::select(m_fd.get() + 1, &readfds, nullptr, nullptr, (time == -1) ? nullptr : &tv);

        if (result == 0) {
            // Nothin new
            return false;
        } else if (result == 1) {
            // Something happened
            if (FD_ISSET(m_fd.get(), &readfds)) {
                // New data or connection closed
                return true;
            }
        } else {
            // Error
            auto ex = SocketErrorException("select() failed");
            close();
            throw ex;
        }
    }

    return false;
}

void appfw::TcpClient4::close() {
    m_fd.close();
    m_Status = NetClientStatus::Closed;
    m_Addr = SockAddr4();
}

int appfw::TcpClient4::read(appfw::span<uint8_t> buf) {
    int size = ::recv(m_fd.get(), reinterpret_cast<char *>(buf.data()), buf.size(), 0);

    if (size >= 0) {
        return size;
    } else {
        return handleError("recv");
    }
}

int appfw::TcpClient4::write(appfw::span<const uint8_t> buf) {
    int size = ::send(m_fd.get(), reinterpret_cast<const char *>(buf.data()), buf.size(), 0);

    if (size >= 0) {
        return size;
    } else {
        return handleError("send");
    }
}

void appfw::TcpClient4::writeAll(appfw::span<const uint8_t> buf) {
    size_t sent = 0;

    do {
        sent += write(buf.subspan(sent));
    } while (sent != buf.size());
}

int appfw::TcpClient4::handleError(std::string_view callName) {
#if PLATFORM_WINDOWS
    int error = WSAGetLastError();

    if (error != WSAEWOULDBLOCK) {
        close();
        throw SocketErrorException(std::string(callName) + "() failed", error);
    }

    return 0;

#elif PLATFORM_UNIX
    int error = errno;

    static_assert(EAGAIN == EWOULDBLOCK, "EAGAIN and EWOULDBLOCK are different");

    if (error != EWOULDBLOCK) {
        close();
        throw SocketErrorException(std::string(callName) + "() failed", error);
    }

    return 0;
#else
#error
#endif
}
