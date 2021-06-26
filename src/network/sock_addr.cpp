#include <appfw/network/sock_addr.h>
#include "plat_sockets.h"

std::string appfw::SockAddr4::toString() const {
    return fmt::format("{}.{}.{}.{}:{}", (ip.addr & 0xFF000000) >> 24, (ip.addr & 0x00FF0000) >> 16,
                       (ip.addr & 0x0000FF00) >> 8, (ip.addr & 0x000000FF) >> 0, port);
}

sockaddr_in appfw::SockAddr4::toSockAddrStruct() const {
    sockaddr_in sockAddr = {};
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
#if PLATFORM_WINDOWS
    sockAddr.sin_addr.S_un.S_addr = htonl(ip.addr);
#else
    sockAddr.sin_addr.s_addr = htonl(ip.addr);
#endif
    return sockAddr;
}

appfw::SockAddr4 appfw::SockAddr4::fromSockAddrStruct(const sockaddr_in &sa) {
    uint16_t port = ntohs(sa.sin_port);
#if PLATFORM_WINDOWS
    uint32_t ip = ntohl(sa.sin_addr.S_un.S_addr);
#else
    uint32_t ip = ntohl(sa.sin_addr.s_addr);
#endif
    return SockAddr4{ip, port};
}

std::list<appfw::SockAddr4> appfw::resolveHostName(const std::string &hostname,
                                                   const std::string &port, SockType type) {
    platsock::initNetworking();

    addrinfo hints;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    
    if (type == SockType::TCP) {
        hints.ai_socktype = SOCK_STREAM;
    } else if (type == SockType::UDP) {
        hints.ai_socktype = SOCK_DGRAM;
    }

    struct addrinfo *servinfo = nullptr;
    

    try {
        int result = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &servinfo);
        if (result != 0) {
#if PLATFORM_WINDOWS
            throw SocketErrorException("getaddrinfo() failed");
#else
            throw std::runtime_error(gai_strerror(result));
#endif
        }

        std::list<SockAddr4> list;

        for (addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
            list.push_back(
                SockAddr4::fromSockAddrStruct(*reinterpret_cast<sockaddr_in *>(p->ai_addr)));
        }

        freeaddrinfo(servinfo);
        return list;
    } catch (...) {
        freeaddrinfo(servinfo);
        throw;
    }
}
