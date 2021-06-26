#ifndef APPFW_NETWORK_TCP_CLIENT4_H
#define APPFW_NETWORK_TCP_CLIENT4_H
#include <appfw/network/socket.h>
#include <appfw/span.h>
#include <appfw/timer.h>

namespace appfw {

class TcpClient4 {
public:
    //! Default time-out in ms
    static constexpr int DEF_TIMEOUT = 5000;

    /**
     * @return The status of the socket
     */
    inline NetClientStatus getStatus() { return m_Status; }

    /**
     * @return The address of remote server
     */
    inline const SockAddr4 &getRemoteAddr() { return m_Addr; }

    /**
     * Connects the client to a server. Throws on error or if not closed.
     * @param   addr    Address of the server
     * @param   timeout Time-out in ms
     */
    void connect(const SockAddr4 &addr, int timeout = DEF_TIMEOUT);

    /**
     * Updates the status of the socket.
     * Throws and closes on error.
     * @param   time    Time to wait in ms (0 - don't wait, -1 - indefinitely)
     * @return  true if data is available to read
     */
    bool updateStatus(int time);

    /**
     * Disconnects and closes the socket.
     */
    void close();

    /**
     * Reads up to N bytes from the socket. Non-blocking.
     * Throws on failure. The socket will be closed in that case.
     * @param   buf     Buffer to put the data into.
     * @return  Number of bytes read. Returns 0 if nothing to read. Returns 0 if EOF.
     */
    int read(appfw::span<uint8_t> buf);

    /**
     * Sends up to N bytes of data in the buffer.
     * Throws on failure. The socket will be closed in that case.
     * @param   buf     Data to send
     * @return  Number of bytes send
     */
    int write(appfw::span<const uint8_t> buf);

    /**
     * Sends all data in the buffer. May block.
     * Throws on failure. The socket will be closed in that case.
     * @param   buf     Data to send
     */
    void writeAll(appfw::span<const uint8_t> buf);

private:
    NetClientStatus m_Status = NetClientStatus::Closed;
    SockAddr4 m_Addr;
    SockFd m_fd;
    int m_iTimeOut = 0;
    Timer m_Timer;

    int handleError(std::string_view callName);
};

} // namespace appfw 

#endif
