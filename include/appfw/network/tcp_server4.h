#ifndef APPFW_NETWORK_TCP_SERVER4_H
#define APPFW_NETWORK_TCP_SERVER4_H
#include <memory>
#include <functional>
#include <appfw/network/socket.h>
#include <appfw/span.h>

namespace appfw {

class TcpClientSocket4;
using TcpClientSocket4Ptr = std::shared_ptr<TcpClientSocket4>;

/**
 * A TCP server for IPv4 with callbacks.
 */
class TcpServer4 {
public:
    using ConnAcceptedCallback = std::function<void(size_t index, TcpClientSocket4Ptr socket)>;
    using ConnClosedCallback = std::function<void(size_t index, TcpClientSocket4Ptr socket, SocketCloseReason reason)>;
    using ReadyReadCallback = std::function<void(size_t index, TcpClientSocket4Ptr socket)>;

    /**
     * Default size of the pending connections queue.
     */
    static constexpr int CONN_QUEUE_SIZE = 16;

    TcpServer4();
    ~TcpServer4();

    /**
     * @return whether the server is listening for incoming connections
     */
    inline bool isListening() { return m_fd.get() != 0; }

    /**
     * Starts listening for incoming connections.
     * Throws if already open or fails to open.
     * @param   ip          IP address of interface (can be ADDR4_ANY)
     * @param   port        Listen port
     * @param   queueSize   Size of incoming ocnnections queue
     */
    void startListening(IPAddress4 ip, uint16_t port, int queueSize = CONN_QUEUE_SIZE);

    /**
     * Stops listening for incoming connections.
     */
    void stopListening();

    /**
     * @return address passed to startListening
     */
    inline const SockAddr4 &getListenAddress() { return m_ListenAddr; }

    /**
     * Accepts incoming connections and reads incoming data.
     * @param   time    Time to wait in ms (0 - don't wait, -1 - indefinitely)
     */
    void poll(int time);

    /**
     * Returns the number of connected clients.
     * This number is updated during poll. Don't save it.
     */
    size_t getConnectedClients();

    /**
     * Returns a client socket.
     * @param   idx     Client idx [0; getConnectedClients())
     */
    TcpClientSocket4Ptr getSocket(size_t idx);

    /**
     * Sends all data in the buffer to all clients. May block.
     * Throws on failure. The socket will be closed in that case.
     * @param   buf     Data to send
     */
    void sendToAll(appfw::span<uint8_t> buf);

    /**
     * Sets callback that is called when a new client is accepted.
     * Must not throw.
     */
    void setConnAcceptedCallback(const ConnAcceptedCallback &fn);

    /**
     * Sets callback that is called when a connection is closed.
     * It is caled from poll() for any socket that is lcosed (even manually).
     * Must not throw.
     */
    void setConnClosedCallback(const ConnClosedCallback &fn);

    /**
     * Sets callback that is called when a data is received.
     * Must not throw.
     */
    void setReadyReadCallback(const ReadyReadCallback &fn);

private:
    struct Data;

    appfw::SockFd m_fd;
    SockAddr4 m_ListenAddr = SockAddr4();
    std::unique_ptr<Data> m_Data;
    ConnAcceptedCallback m_fnAcceptedCb;
    ConnClosedCallback m_fnClosedCb;
    ReadyReadCallback m_fnReadyReadCb;

    void acceptConnections();
    void acceptConnection(SocketFile sock, const SockAddr4 &addr);
    void onReadyRead(size_t idx);
    void onConnectionClosed(size_t idx);
};

class TcpClientSocket4 {
public:
    /**
     * @return whether the socket is open or not
     */
    inline bool isOpen() { return m_fd.get() != 0 && !m_bIsClosing; }

    /**
     * Marks the socket as awaiting closing.
     * It will be closed at the end of poll() call.
     */
    inline void close(SocketCloseReason reason = SocketCloseReason::User) {
        m_bIsClosing = true;
        m_CloseReason = reason;
    }

    /**
     * @return remote address
     */
    inline const SockAddr4 &getRemoteAddress() { return m_RemoteAddr; }

    /**
     * Sets user data pointer
     */
    inline void setUserData(void *userdata) { m_pUserData = userdata; }

    /**
     * @return user data set in setUserData
     */
    inline void *getUserData() { return m_pUserData; }

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
    appfw::SockFd m_fd;
    SockAddr4 m_RemoteAddr = SockAddr4();
    bool m_bIsClosing = false;
    void *m_pUserData = nullptr;
    SocketCloseReason m_CloseReason = SocketCloseReason::Failure;

    int handleError(std::string_view callName);

    friend class TcpServer4;
};

} // namespace appfw 

#endif
