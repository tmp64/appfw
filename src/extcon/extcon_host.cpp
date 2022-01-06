#include <appfw/binary_buffer.h>
#include <appfw/extcon/extcon_host.h>
#include <appfw/extcon/consts.h>

appfw::ExtconHost::ExtconHost()
    : m_Worker(*this) {
    
}

appfw::ExtconHost::~ExtconHost() {}

void appfw::ExtconHost::enable(const IPAddress4 &addr, uint16_t port) {
    disable();

    m_Worker.start(addr, port);
    m_bIsWorkerRunning = true;
}

void appfw::ExtconHost::disable() {
    if (m_bIsWorkerRunning) {
        m_Worker.stop();
        m_bStateIsConnected = false;
        m_bIsConnected = false;
    }
}

void appfw::ExtconHost::tick() {
    bool bStateIsConnected;
    {
        std::lock_guard lock(m_StateSyncMutex);
        bStateIsConnected = m_bStateIsConnected;
    }

    if (bStateIsConnected && !m_bIsConnected) {
        m_bIsConnected = true;
        onClientConnected();
    } else if (!bStateIsConnected && m_bIsConnected) {
        m_bIsConnected = false;
        onClientDisconnected();
    }

    // Execute commands
    {
        std::lock_guard lock(m_CommandQueueMutex);
        
        while (!m_CommandQueue.empty()) {
            std::string &cmd = m_CommandQueue.front();
            m_pConSys->command(cmd);
            m_CommandQueue.pop();
        }
    }
}

void appfw::ExtconHost::onAdd(ConsoleSystem *pConSys) {
    AFW_ASSERT(!m_pConSys);
    m_pConSys = pConSys;
}

void appfw::ExtconHost::onRemove([[maybe_unused]] ConsoleSystem *pConSys) {
    AFW_ASSERT(m_pConSys == pConSys);
    m_pConSys = nullptr;
}

void appfw::ExtconHost::print(const ConMsgInfo &info, std::string_view msg) {
    if (!m_bIsConnected) {
        return;
    }
    
    std::unique_lock lock(m_PrintQueueMutex);

    if (m_PrintQueue.size() >= MAX_QUEUE_SIZE) {
        // Block until the queue is empty
        m_PrintQueueEmptyCv.wait(lock, [&]() { return m_PrintQueue.empty(); });
    }

    PrintMessage printMsg{info, std::string(msg)};
    m_PrintQueue.push(std::move(printMsg));
}

bool appfw::ExtconHost::isThreadSafe() {
    return true;
}

void appfw::ExtconHost::onClientConnected() {
    m_pConSys->printPreviousMessages(this);

    {
        std::lock_guard lock(m_AvailableCommandsMutex);
        auto &items = m_pConSys->getItemMap();
        m_AvailableCommands.clear();
        m_AvailableCommands.reserve(items.size());

        for (const auto &i : items) {
            m_AvailableCommands.push_back(i.first);
        }
    }
}

void appfw::ExtconHost::onClientDisconnected() {}

appfw::ExtconHost::WorkerThread::WorkerThread(ExtconHost &con)
    : m_Con(con) {
    m_Server.setConnAcceptedCallback(
        [&](auto, auto socket) { onConnAccepted(socket); });
    m_Server.setConnClosedCallback(
        [&](auto, auto socket, auto reason) { onConnClosed(socket, reason); });
    m_Server.setReadyReadCallback([&](auto, auto socket) { onReadyRead(socket); });

    m_ClientParser.setMagic(EXTCON_MSG_MAGIC, EXTCON_MSG_MAGIC_SIZE);
    m_ClientParser.setMaxPayloadSize(EXTCON_MAX_PAYLOAD_SIZE);
    m_ClientParser.setPayloadCallback(
        [&](appfw::BinaryInputStream &stream, uint8_t *, size_t) { onPayloadReceived(stream); });

    m_Buffer.resize(MAX_TCP_READ_SIZE);
}

appfw::ExtconHost::WorkerThread::~WorkerThread() {
    AFW_ASSERT_REL(!m_bIsThreadRunning);
}

void appfw::ExtconHost::WorkerThread::start(const IPAddress4 &addr, uint16_t port) {
    AFW_ASSERT_REL(!m_bIsThreadRunning);
    m_bIsThreadRunning = true;
    m_Thread = std::thread([=]() { run(addr, port); });
}

void appfw::ExtconHost::WorkerThread::stop() {
    m_bIsThreadRunning = false;
    m_Thread.join();
}

void appfw::ExtconHost::WorkerThread::run(const IPAddress4 &addr, uint16_t port) noexcept {
    try {
        m_Server.startListening(addr, port, 1);
    } catch (const std::exception e) {
        printe("extcon: Failed to start the server: {}", e.what());
        return;
    }

    try {
        while (m_bIsThreadRunning) {
            pollServer();
            updateConnectedClient();
        }
    } catch (const std::exception e) {
        printe("extcon: Server error: {}", e.what());
        return;
    }
}

void appfw::ExtconHost::WorkerThread::pollServer() {
    m_Server.poll(POLL_TIME);
}

void appfw::ExtconHost::WorkerThread::updateConnectedClient() {
    if (!m_bIsSocketValid) {
        return;
    }

    try {
        sendAvailableCommands();
        sendQueuedMessages();
        sendRequestFocus();
    } catch (const std::exception &e) {
        printe("extcon: Write failed: {}", e.what());
    }
}

void appfw::ExtconHost::WorkerThread::sendAvailableCommands() {
    std::vector<std::string> commands;

    {
        std::lock_guard lock(m_Con.m_AvailableCommandsMutex);

        if (m_Con.m_AvailableCommands.empty()) {
            return;
        }

        commands.swap(m_Con.m_AvailableCommands);
    }

    size_t i = 0;

    while (i < commands.size()) {
        appfw::BinaryBuffer stream = prepareSendBuffer(EXTCON_OPCODE_CMD_LIST);
        uint32_t countInThisMessage = 0;
        uint32_t payloadSize = 0;

        appfw::binpos countPos = stream.getPosition();
        stream.writeUInt32(0); // Number of commands

        for (; i < commands.size(); i++) {
            const std::string &cmd = commands[i];
            uint32_t size = (uint32_t)(sizeof(uint32_t) + cmd.size());

            if (payloadSize + size > EXTCON_MAX_PAYLOAD_SIZE) {
                // This command doesn't fit, needs to be sent in the next message
                break;
            } else {
                stream.writeString(cmd);
                payloadSize += size;
                countInThisMessage++;
            }
        }

        // Write the count
        appfw::binpos endPos = stream.getPosition();
        stream.seekAbsolute(countPos);
        stream.writeUInt32(countInThisMessage);
        stream.seekAbsolute(endPos);

        // Send it
        sendBuffer(stream);
    }
}

void appfw::ExtconHost::WorkerThread::sendQueuedMessages() {
    std::unique_lock lock(m_Con.m_PrintQueueMutex);
    auto &queue = m_Con.m_PrintQueue;

    while (!queue.empty()) {
        PrintMessage msg = std::move(queue.front());
        queue.pop();
        lock.unlock();

        BinaryBuffer stream = prepareSendBuffer(EXTCON_OPCODE_PRINT);
        stream.writeByte((uint8_t)msg.info.type);
        stream.writeByte((uint8_t)msg.info.color);
        stream.writeInt64(msg.info.time);
        stream.writeString(msg.info.tag);
        stream.writeString(msg.text);
        sendBuffer(stream);

        lock.lock();
    }

    if (queue.empty()) {
        lock.unlock();
        m_Con.m_PrintQueueEmptyCv.notify_all();
    }
}

void appfw::ExtconHost::WorkerThread::sendRequestFocus() {
    if (m_Con.m_bClientFocusRequested.exchange(false)) {
        appfw::BinaryBuffer stream = prepareSendBuffer(EXTCON_OPCODE_REQUEST_FOCUS);
        sendBuffer(stream);
    }
}

void appfw::ExtconHost::WorkerThread::onConnAccepted(appfw::TcpClientSocket4Ptr socket) noexcept {
    if (!m_pClientSocket) {
        // Accept the connection
        m_pClientSocket = socket;
        m_ClientParser.reset();
        m_bIsSocketValid = true;

        std::lock_guard lockState(m_Con.m_StateSyncMutex);
        m_Con.m_bStateIsConnected = true;
        m_Con.m_bClientFocusRequested = false;
        m_Con.m_bHostFocusRequested = false;

        // Clear msg queue
        std::lock_guard lockPrint(m_Con.m_PrintQueueMutex);
        while (!m_Con.m_PrintQueue.empty()) {
            m_Con.m_PrintQueue.pop();
        }
    } else {
        // Only one client is supported, close the connection
        socket->close();
    }
}

void appfw::ExtconHost::WorkerThread::onConnClosed(appfw::TcpClientSocket4Ptr socket,
                                                   appfw::SocketCloseReason reason) noexcept {
    if (m_pClientSocket == socket) {
        m_pClientSocket = nullptr;
        m_bIsSocketValid = false;

        std::string_view reasonStr;

        switch (reason) {
        case appfw::SocketCloseReason::Failure:
            reasonStr = "Failure";
            break;
        case appfw::SocketCloseReason::ConnAborted:
            reasonStr = "Connection aborted";
            break;
        case appfw::SocketCloseReason::TimeOut:
            reasonStr = "Timed out";
            break;
        case appfw::SocketCloseReason::User:
            reasonStr = "Closed by user";
            break;
        case appfw::SocketCloseReason::Shutdown:
            reasonStr = "Server is going down";
            break;
        default:
            reasonStr = "Unknown";
        }

        printn("extcon: Client disconnected {}: {}", socket->getRemoteAddress().toString(),
               reasonStr);

        std::lock_guard lock(m_Con.m_StateSyncMutex);
        m_Con.m_bStateIsConnected = false;
    }
}

void appfw::ExtconHost::WorkerThread::onReadyRead(appfw::TcpClientSocket4Ptr socket) noexcept {
    int readSize = 0;

    try {
        readSize = socket->read(m_Buffer);
    } catch (const std::exception &e) {
        m_bIsSocketValid = false;
        printe("extcon: Read failed: {}", e.what());
        return;
    }

    if (readSize <= 0) {
        m_bIsSocketValid = false;
        socket->close(appfw::SocketCloseReason::ConnAborted);
        return;
    }

    uint32_t bytesParsed = 0;
    DatagramParser::Error error;

    if (!m_ClientParser.parseData(m_Buffer.data(), readSize, bytesParsed, error)) {
        m_bIsSocketValid = false;
        socket->close();
        printe("extcon: Received invalid data");
    } else if (bytesParsed != (uint32_t)readSize) {
        m_bIsSocketValid = false;
        socket->close();
        printe("extcon: Not all data was parsed");
    }
}

void appfw::ExtconHost::WorkerThread::onPayloadReceived(appfw::BinaryInputStream &stream) noexcept {
    try {
        uint8_t opcode = stream.readByte();

        switch (opcode) {
        case EXTCON_OPCODE_COMMAND: {
            std::string command = stream.readString();
            std::lock_guard lock(m_Con.m_CommandQueueMutex);

            // Echo to the console
            ConMsgInfo info;
            info.setType(ConMsgType::Input).setTag("extcon");
            m_Con.m_pConSys->print(info, "> " + command);

            // Push to the queue
            m_Con.m_CommandQueue.push(std::move(command));
            break;
        }
        case EXTCON_OPCODE_REQUEST_FOCUS: {
            m_Con.m_bHostFocusRequested = true;
            break;
        }
        default: {
            throw std::runtime_error(fmt::format("invalid opcode {}", opcode));
        }
        }
    } catch (const std::exception &e) {
        printe("extcon: Invalid payload: {}", e.what());
    }
}

appfw::BinaryBuffer appfw::ExtconHost::WorkerThread::prepareSendBuffer(uint8_t opcode) {
    BinaryBuffer stream(m_Buffer);
    stream.writeBytes(EXTCON_MSG_MAGIC, EXTCON_MSG_MAGIC_SIZE);
    stream.writeUInt32(0); // payload size
    stream.writeByte(opcode);
    return stream;
}

void appfw::ExtconHost::WorkerThread::sendBuffer(appfw::BinaryBuffer &stream) {
    uint32_t packetSize = (uint32_t)stream.getPosition();
    uint32_t payloadSize = packetSize - EXTCON_MSG_MAGIC_SIZE - sizeof(uint32_t);
    stream.seekAbsolute(EXTCON_MSG_MAGIC_SIZE);
    stream.writeUInt32(payloadSize);

    m_pClientSocket->write(appfw::span(m_Buffer.data(), packetSize));
}
