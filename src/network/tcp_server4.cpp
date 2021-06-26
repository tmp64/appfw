#include <vector>
#include <appfw/network/tcp_server4.h>
#include "plat_sockets.h"

//----------------------------------------------------------------
// TcpServer4::Data
//----------------------------------------------------------------
struct appfw::TcpServer4::Data {
    std::vector<pollfd> m_PollList;
    std::vector<TcpClientSocket4Ptr> m_Sockets;
};

//----------------------------------------------------------------
// TcpServer4
//----------------------------------------------------------------
appfw::TcpServer4::TcpServer4() = default;

appfw::TcpServer4::~TcpServer4() = default;

void appfw::TcpServer4::startListening(IPAddress4 ip, uint16_t port, int queueSize) {
    if (isListening()) {
        throw std::logic_error("already listening");
    }

    try {
        platsock::initNetworking();

        int result = 0;
        m_ListenAddr = SockAddr4{ip, port};
        sockaddr_in sockAddr = m_ListenAddr.toSockAddrStruct();

        // Open the socket
        {
            SocketFile sockfd = ::socket(PF_INET, SOCK_STREAM, 0);

            if (sockfd == NULL_SOCKET) {
                throw SocketErrorException("socket() failed");
            }

            m_fd.set(sockfd);
        }

        // Set non-blocking mode
        if (!platsock::setSocketBlockingMode(m_fd.get(), false)) {
            throw SocketErrorException("setSocketBlockingMode(false) failed");
        }

        // Bind it
        result = ::bind(m_fd.get(), reinterpret_cast<sockaddr *>(&sockAddr), sizeof(sockAddr));
        if (result != 0) {
            throw SocketErrorException("bind() failed");
        }

        // Start listening 
        result = ::listen(m_fd.get(), queueSize);
        if (result != 0) {
            throw SocketErrorException("listen() failed");
        }

        // Create internal data instance
        m_Data = std::make_unique<Data>();

        // Push server socket into poll list
        m_Data->m_PollList.push_back({m_fd.get(), POLLIN, 0});
    } catch (...) {
        stopListening();
        throw;
    }
}

void appfw::TcpServer4::stopListening() {
    // Close all active connections
    for (size_t i = 0; i < m_Data->m_Sockets.size(); i++) {
        m_Data->m_Sockets[i]->close(SocketCloseReason::Shutdown);
        m_Data->m_Sockets[i]->m_fd.close();
        onConnectionClosed(i);
    }

    m_Data.reset();
    m_fd.close();
}

void appfw::TcpServer4::poll(int time) {
    if (!isListening()) {
        throw std::logic_error("not listening");
    }

    AFW_ASSERT(m_Data->m_PollList.size() >= 1);

    int num = appfw::platsock::poll(m_Data->m_PollList.data(), m_Data->m_PollList.size(), time);

    if (num < 0) {
        // Error, most likely unrecoverable
        auto ex = SocketErrorException("poll() failed");
        stopListening();
        throw ex;
    } else if (num > 0) {
        // Check for new connections
        {
            int revents = m_Data->m_PollList[0].revents;

            if (revents & POLLERR || revents & POLLHUP || revents & POLLNVAL) {
                // Listen socket failed
                auto ex = SocketErrorException("listen socket failed");
                stopListening();
                throw ex;
            }

            if (revents & POLLIN) {
                // Accept new connections
                acceptConnections();
                num--;
            }
        }

        // Read data
        for (size_t i = 1; i < m_Data->m_PollList.size() && num > 0; i++) {
            int revents = m_Data->m_PollList[i].revents;
            if (revents & POLLHUP) {
                m_Data->m_Sockets[i - 1]->close(SocketCloseReason::ConnAborted);
                num--;
            } else if (revents & POLLERR || revents & POLLNVAL) {
                m_Data->m_Sockets[i - 1]->close(SocketCloseReason::Failure);
                num--;
            } else if (revents & POLLIN) {
                if (m_Data->m_Sockets[i - 1]->isOpen()) {
                    onReadyRead(i - 1);
                }
                num--;
            }
        }
    }

    // Remove closed sockets
    for (size_t i = m_Data->m_Sockets.size(); i != 0; i--) {
        if (!m_Data->m_Sockets[i - 1]->isOpen()) {
            m_Data->m_Sockets[i - 1]->m_fd.close();
            onConnectionClosed(i - 1);

            // Remove socket from lists
            m_Data->m_Sockets.erase(m_Data->m_Sockets.begin() + i - 1);
            m_Data->m_PollList.erase(m_Data->m_PollList.begin() + i);
        }
    }  
}

size_t appfw::TcpServer4::getConnectedClients() {
    return m_Data->m_Sockets.size();
}

appfw::TcpClientSocket4Ptr appfw::TcpServer4::getSocket(size_t idx) {
    return m_Data->m_Sockets[idx];
}

void appfw::TcpServer4::sendToAll(appfw::span<uint8_t> buf) {
    for (size_t i = 0; i < m_Data->m_Sockets.size(); i++) {
        m_Data->m_Sockets[i]->writeAll(buf);
    }
}

void appfw::TcpServer4::setConnAcceptedCallback(const ConnAcceptedCallback &fn) {
    m_fnAcceptedCb = fn;
}

void appfw::TcpServer4::setConnClosedCallback(const ConnClosedCallback &fn) {
    m_fnClosedCb = fn;
}

void appfw::TcpServer4::setReadyReadCallback(const ReadyReadCallback &fn) {
    m_fnReadyReadCb = fn;
}

void appfw::TcpServer4::acceptConnections() {
    SocketFile sock = 0;
    SockAddr4 addr;
    platsock::AcceptResult result = platsock::AcceptResult::Accepted;

    do {
        result = platsock::acceptConnection(m_fd.get(), sock, addr);

        switch (result) {
        case platsock::AcceptResult::Accepted: {
            acceptConnection(sock, addr);
            break;
        }
        case platsock::AcceptResult::BadAccept: {
            // Ignore this one
            result = platsock::AcceptResult::WouldBlock;
            continue;
        }
        }
    } while (result != platsock::AcceptResult::WouldBlock);
}

void appfw::TcpServer4::acceptConnection(SocketFile sock, const SockAddr4 &addr) {
    platsock::setSocketBlockingMode(sock, false);
    size_t index = m_Data->m_Sockets.size();
    m_Data->m_Sockets.push_back(std::make_shared<TcpClientSocket4>());

    auto &socket = m_Data->m_Sockets[index];
    socket->m_fd.set(sock);
    socket->m_RemoteAddr = addr;

    // Add it to poll list
    m_Data->m_PollList.push_back({sock, POLLIN, 0});

    try {
        m_fnAcceptedCb(index, socket);
    } catch (...) {
        AFW_ASSERT_REL_MSG(false, "Callback must not throw");
        std::abort();
    }
}

void appfw::TcpServer4::onReadyRead(size_t idx) {
    try {
        m_fnReadyReadCb(idx, m_Data->m_Sockets[idx]);
    } catch (...) {
        AFW_ASSERT_REL_MSG(false, "Callback must not throw");
        std::abort();
    }
}

void appfw::TcpServer4::onConnectionClosed(size_t idx) {
    try {
        auto &socket = m_Data->m_Sockets[idx];
        m_fnClosedCb(idx, socket, socket->m_CloseReason);
    } catch (...) {
        AFW_ASSERT_REL_MSG(false, "Callback must not throw");
        std::abort();
    }
}

//----------------------------------------------------------------
// TcpClientSocket4
//----------------------------------------------------------------
int appfw::TcpClientSocket4::read(appfw::span<uint8_t> buf) {
    int size = ::recv(m_fd.get(), reinterpret_cast<char *>(buf.data()), buf.size(), 0);

    if (size >= 0) {
        return size;
    } else {
        return handleError("recv");
    }
}

int appfw::TcpClientSocket4::write(appfw::span<const uint8_t> buf) {
    int size = ::send(m_fd.get(), reinterpret_cast<const char *>(buf.data()), buf.size(), 0);

    if (size >= 0) {
        return size;
    } else {
        return handleError("send");
    }
}

void appfw::TcpClientSocket4::writeAll(appfw::span<const uint8_t> buf) {
    size_t sent = 0;

    do {
        sent += write(buf.subspan(sent));
    } while (sent != buf.size());
}

int appfw::TcpClientSocket4::handleError(std::string_view callName) {
#if PLATFORM_WINDOWS
    int error = WSAGetLastError();

    switch (error) {
    case WSAEWOULDBLOCK: {
        return 0;
    }
    case WSAECONNABORTED: {
        close(SocketCloseReason::ConnAborted);
        break;
    }
    case WSAENETRESET:
    case WSAETIMEDOUT: {
        close(SocketCloseReason::TimeOut);
        break;
    }
    default: {
        close(SocketCloseReason::Failure);
    }
    }

    throw SocketErrorException(std::string(callName) + "() failed", error);
#elif PLATFORM_UNIX
    int error = errno;

    static_assert(EAGAIN == EWOULDBLOCK, "EAGAIN and EWOULDBLOCK are different");

    switch (errno) {
    case EWOULDBLOCK: {
        return 0;
    }
    case ECONNREFUSED: {
        close(SocketCloseReason::ConnAborted);
        break;
    }

    default: {
        close(SocketCloseReason::Failure);
    }
    }

    throw SocketErrorException(std::string(callName) + "() failed", error);
#else
#error
#endif
}
