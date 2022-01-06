#ifndef APPFW_EXTCON_EXTCON_HOST_H
#define APPFW_EXTCON_EXTCON_HOST_H
#include <condition_variable>
#include <appfw/binary_buffer.h>
#include <appfw/console/console_system.h>
#include <appfw/network/tcp_server4.h>
#include <appfw/network/datagram_parser.h>

namespace appfw {

class ExtconHost : public appfw::IConsoleReceiver, appfw::NoMove {
public:
    ExtconHost();
    ~ExtconHost();

    inline bool isEnabled() { return m_bIsWorkerRunning; }
    inline bool isConnected() { return m_bIsConnected; }

    //! Begins listening for extcon connections on specified address.
    void enable(const IPAddress4 &addr, uint16_t port);

    //! Disconnects all clients and closes the server socket.
    void disable();

    //! Checks for incoming connections.
    void tick();

    //! Requests client focus.
    inline void requestCientFocus() { m_bClientFocusRequested = true; }

    //! @returns whether host focus is requested and resets the flag.
    inline bool isHostFocusRequested() { return m_bHostFocusRequested.exchange(false); }

    // IConsoleReceiver
    void onAdd(ConsoleSystem *pConSys) override;
    void onRemove(ConsoleSystem *pConSys) override;
    void print(const ConMsgInfo &info, std::string_view msg) override;
    bool isThreadSafe() override;

private:
    //! Maximum queue size. If it's more than that, print call will wait until it's empty.
    static constexpr int MAX_QUEUE_SIZE = 128;

    class WorkerThread : appfw::NoMove {
    public:
        WorkerThread(ExtconHost &con);
        ~WorkerThread();

        void start(const IPAddress4 &addr, uint16_t port);
        void stop();

    private:
        static constexpr int POLL_TIME = 1000 / 60;
        static constexpr size_t MAX_TCP_READ_SIZE = 65535;

        ExtconHost &m_Con;
        std::atomic_bool m_bIsThreadRunning = false;
        std::thread m_Thread;

        TcpServer4 m_Server;
        DatagramParser m_ClientParser;
        std::vector<uint8_t> m_Buffer;

        appfw::TcpClientSocket4Ptr m_pClientSocket;
        bool m_bIsSocketValid = false;

        void run(const IPAddress4 &addr, uint16_t port) noexcept;
        void pollServer();
        void updateConnectedClient();
        void sendAvailableCommands();
        void sendQueuedMessages();
        void sendRequestFocus();
        void onConnAccepted(appfw::TcpClientSocket4Ptr socket) noexcept;
        void onConnClosed(appfw::TcpClientSocket4Ptr socket,
                          appfw::SocketCloseReason reason) noexcept;
        void onReadyRead(appfw::TcpClientSocket4Ptr socket) noexcept;
        void onPayloadReceived(appfw::BinaryInputStream &stream) noexcept;
        appfw::BinaryBuffer prepareSendBuffer(uint8_t opcode);
        void sendBuffer(appfw::BinaryBuffer &buffer);
    };

    struct PrintMessage {
        ConMsgInfo info;
        std::string text;
    };

    ConsoleSystem *m_pConSys = nullptr;
    WorkerThread m_Worker;
    bool m_bIsWorkerRunning = false;
    bool m_bIsConnected = false;

    std::mutex m_StateSyncMutex; // State variables are updated from the worker
    bool m_bStateIsConnected = false;

    std::mutex m_PrintQueueMutex;
    std::condition_variable m_PrintQueueEmptyCv;
    std::queue<PrintMessage> m_PrintQueue;

    std::mutex m_AvailableCommandsMutex;
    std::vector<std::string> m_AvailableCommands;

    std::mutex m_CommandQueueMutex;
    std::queue<std::string> m_CommandQueue;

    std::atomic_bool m_bClientFocusRequested = false;
    std::atomic_bool m_bHostFocusRequested = false;

    void onClientConnected();
    void onClientDisconnected();
};

} // namespace appfw 

#endif
