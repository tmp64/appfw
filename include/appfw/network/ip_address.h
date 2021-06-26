#ifndef APPFW_NETWORK_IP_ADDRESS_H
#define APPFW_NETWORK_IP_ADDRESS_H
#include <cstdint>

namespace appfw {

struct IPAddress4 {
    //! IP address in host byte order
    uint32_t addr;

    IPAddress4() = default;

    /**
     * Constructs an IP address from an integer.
     * Integer is interpreted in host byte order
     * 127.0.0.1 == 0x7F00'0001
     */
    constexpr inline IPAddress4(uint32_t a) : addr(a) {}
};

static constexpr IPAddress4 ADDR4_ANY = 0x0000'0000;
static constexpr IPAddress4 ADDR4_LOOPBACK = 0x7F00'0001;
static constexpr IPAddress4 ADDR4_BROADCAST = 0xFFFF'FFFF;

} // namespace appfw 

#endif
