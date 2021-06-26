#ifndef APPFW_NETWORK_SOCK_ADD_H
#define APPFW_NETWORK_SOCK_ADD_H
#include <string>
#include <list>
#include <appfw/network/ip_address.h>

struct sockaddr_in;

namespace appfw {

struct SockAddr4 {
    //! IP address
    IPAddress4 ip;

    //! Port in host byte order
    uint16_t port;

    //! Converts address to a string
    std::string toString() const;

    //! Converts the address to struct sockaddr_in.
    sockaddr_in toSockAddrStruct() const;

    //! Converts struct sockaddr_in to SockAddr4
    static SockAddr4 fromSockAddrStruct(const sockaddr_in &sa);
};

enum class SockType
{
    Any = 0,
    TCP,
    UDP
};

/**
 * Resolves hostname and port into a list of addresses.
 * See getaddrinfo(3).
 */
std::list<SockAddr4> resolveHostName(const std::string &hostname, const std::string &port,
                                     SockType type = SockType::Any);

} // namespace appfw 

#endif
