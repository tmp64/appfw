#ifndef APPFW_NETWORK_PLAT_SOCKETS_H
#define APPFW_NETWORK_PLAT_SOCKETS_H
#include <appfw/network/socket.h>

#if PLATFORM_WINDOWS

#include <appfw/windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#elif PLATFORM_UNIX

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

#else
#error "Sockets are not supported on the platform"
#endif

namespace appfw::platsock {

enum class AcceptResult
{
    BadAccept,  //!< Connection closed before accept
    WouldBlock, //!< Nothing left to accept
    Accepted,   //!< Accepted
};

/**
 * Initializes networking. Only needed for WinSock.
 */
void initNetworking();

/**
 * Attempts to accept a connection.
 * Returns true if successfully
 */
AcceptResult acceptConnection(SocketFile fd, SocketFile &remoteSock, SockAddr4 &remoteAddr);

/**
 * Sets socket mode to blocking or non-blocking.
 * @param   socket  A valid socket file descriptor
 * @param   block   Whether to block or not
 * @return  true if successfull
 */
bool setSocketBlockingMode(SocketFile socket, bool block);

/**
 * See poll(2)
 */
int poll(pollfd *ufds, unsigned int nfds, int timeout);

} // namespace appfw::platsock


#endif
