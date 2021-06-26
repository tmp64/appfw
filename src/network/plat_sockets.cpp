#include <appfw/dbg.h>
#include "plat_sockets.h"

#if PLATFORM_WINDOWS

static_assert(std::is_same_v<appfw::SocketFile, SOCKET>, "SocketFile != SOCKET");
static_assert(appfw::NULL_SOCKET == INVALID_SOCKET, "NULL_SOCKET != INVALID_SOCKET");

namespace {

struct WSAInit {
    bool isInited = false;

    void init() {
        if (!isInited) {
            WORD wVersionRequested = MAKEWORD(2, 2);
            WSADATA wsaData;

            int err = ::WSAStartup(wVersionRequested, &wsaData);
            if (err != 0) {
                throw appfw::SocketErrorException("WSAStartup failed", err);
            }

            isInited = true;
        }
    }

    ~WSAInit() {
        if (isInited) {
            ::WSACleanup();
            isInited = false;
        }
    }
};

WSAInit s_WSAInit;

} // namespace

void appfw::platsock::initNetworking() {
    s_WSAInit.init();
}

appfw::platsock::AcceptResult
appfw::platsock::acceptConnection(SocketFile fd, SocketFile &remoteSock,
                                                              SockAddr4 &remoteAddr) {
    sockaddr_storage remoteSockAddr;
    memset(&remoteSockAddr, 0, sizeof(remoteSockAddr));
    int remoteAddrSize = sizeof(remoteSockAddr);

    SocketFile sock = ::accept(fd, reinterpret_cast<sockaddr *>(&remoteSockAddr), &remoteAddrSize);

    if (sock == NULL_SOCKET) {
        int error = WSAGetLastError();

        switch (error) {
        case WSAEWOULDBLOCK: {
            return AcceptResult::WouldBlock;
        }
        case WSAECONNRESET: {
            return AcceptResult::BadAccept;
        }
        default: {
            throw SocketErrorException("accept() failed");
        }
        }
    }

    AFW_ASSERT(remoteSockAddr.ss_family == AF_INET);
    remoteSock = sock;
    remoteAddr = SockAddr4::fromSockAddrStruct(*reinterpret_cast<sockaddr_in *>(&remoteSockAddr));
    return AcceptResult::Accepted;
}

bool appfw::platsock::setSocketBlockingMode(SocketFile socket, bool block) {
    AFW_ASSERT(socket > 0);
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
    u_long iMode = !block;
    return ioctlsocket(socket, FIONBIO, &iMode) == 0;
}

int appfw::platsock::poll(pollfd *ufds, unsigned int nfds, int timeout) {
    return ::WSAPoll(ufds, nfds, timeout);
}

#elif PLATFORM_UNIX

static_assert(std::is_same_v<appfw::SocketFile, int>, "SocketFile != int");
static_assert(appfw::NULL_SOCKET == -1, "NULL_SOCKET != -1");

void appfw::platsock::initNetworking() {
    // Do nothing.
}

appfw::platsock::AcceptResult
appfw::platsock::acceptConnection(SocketFile fd, SocketFile &remoteSock,
                                                              SockAddr4 &remoteAddr) {
    sockaddr_storage remoteSockAddr;
    memset(&remoteSockAddr, 0, sizeof(remoteSockAddr));
    socklen_t remoteAddrSize = sizeof(remoteSockAddr);

    SocketFile sock = ::accept(fd, reinterpret_cast<sockaddr *>(&remoteSockAddr), &remoteAddrSize);

    if (sock == NULL_SOCKET) {
        int error = errno;

        static_assert(EAGAIN == EWOULDBLOCK, "EAGAIN and EWOULDBLOCK are different");

        switch (error) {
        case EWOULDBLOCK: {
            return AcceptResult::WouldBlock;
        }
        case ECONNABORTED: {
            return AcceptResult::BadAccept;
        }
        default: {
            throw SocketErrorException("accept() failed");
        }
        }
    }

    AFW_ASSERT(remoteSockAddr.ss_family == AF_INET);
    remoteSock = sock;
    remoteAddr = SockAddr4::fromSockAddrStruct(*reinterpret_cast<sockaddr_in *>(&remoteSockAddr));
    return AcceptResult::Accepted;
}

bool appfw::platsock::setSocketBlockingMode(SocketFile socket, bool block) {
    AFW_ASSERT(socket > 0);

    int flags = fcntl(socket, F_GETFL);
    if (flags == -1) {
        return false;
    }

    flags = block ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

    int ret = fcntl(socket, F_SETFL, flags);
    if (ret == -1) {
        return false;
    }

    return true;
}

int appfw::platsock::poll(pollfd *ufds, unsigned int nfds, int timeout) {
    return ::poll(ufds, nfds, timeout);
}

#else
#error "Sockets are not supported on the platform"
#endif
