#include <thread>
#include <chrono>
#include <appfw/appfw.h>
#include <appfw/init.h>
#include <appfw/network/tcp_server4.h>

int g_iNextId = 1;

struct ClientData {
    int id = g_iNextId++;
};

appfw::TcpServer4 g_Server;

ConVar<bool> run_app("run_app", true, "Whether or not the app should be running");
ConCommand cmd_quit("quit", "Quits the app", []() { run_app.setValue(false); });

ConCommand cmd_clients("clients", "Lists connected clients", []() {
    size_t count = g_Server.getConnectedClients();

    for (size_t i = 0; i < count; i++) {
        appfw::TcpClientSocket4Ptr socket = g_Server.getSocket(i);
        auto *data = static_cast<ClientData *>(socket->getUserData());
        printi("{}. {}, id = {}", i + 1, socket->getRemoteAddress().toString(), data->id);
    }

    printi("Total: {}", count);
});

ConCommand cmd_send("send", "Sends a message to a client", [](const CmdString &args) {
    size_t count = g_Server.getConnectedClients();

    if (args.size() != 3) {
        printi("Usage: send <id> \"<msg>\"");
        return;
    }

    int id = 0;
    if (!appfw::convertStringToVal(args[1], id)) {
        printe("Invalid id");
        return;
    }

    for (size_t i = 0; i < count; i++) {
        appfw::TcpClientSocket4Ptr socket = g_Server.getSocket(i);
        auto *data = static_cast<ClientData *>(socket->getUserData());
        
        if (data->id == id) {
            try {
                socket->writeAll(appfw::span((uint8_t *)args[2].data(), args[2].size()));
                socket->writeAll(appfw::span((uint8_t *)"\n", 1));
                printi("Message sent");
            } catch (const appfw::NetworkErrorException &e) {
                printe("Send failed: {}", e.what());
            }

            return;
        }
    }

    printe("ID not found");
});

ConCommand cmd_kick("kick", "Kicks a client", [](const CmdString &args) {
    size_t count = g_Server.getConnectedClients();

    if (args.size() != 2) {
        printi("Usage: kick <id>");
        return;
    }

    int id = 0;
    if (!appfw::convertStringToVal(args[1], id)) {
        printe("Invalid id");
        return;
    }

    for (size_t i = 0; i < count; i++) {
        appfw::TcpClientSocket4Ptr socket = g_Server.getSocket(i);
        auto *data = static_cast<ClientData *>(socket->getUserData());

        if (data->id == id) {
            socket->close();
            return;
        }
    }

    printe("ID not found");
});

void onConnAccepted(size_t index, appfw::TcpClientSocket4Ptr socket) noexcept {
    ClientData *data = new ClientData();
    socket->setUserData(data);
    printw("Client connected: {}, id {}", socket->getRemoteAddress().toString(), data->id);
}

void onConnClosed(size_t index, appfw::TcpClientSocket4Ptr socket,
                  appfw::SocketCloseReason reason) noexcept {
    auto *data = static_cast<ClientData *>(socket->getUserData());
    printw("Client disconnected: {}, id {}", socket->getRemoteAddress().toString(), data->id);

    switch (reason) {
    case appfw::SocketCloseReason::Failure:
        printw("Reason: failure");
        break;
    case appfw::SocketCloseReason::ConnAborted:
        printw("Reason: connection aborted");
        break;
    case appfw::SocketCloseReason::TimeOut:
        printw("Reason: timed out");
        break;
    case appfw::SocketCloseReason::User:
        printw("Reason: closed by user");
        break;
    case appfw::SocketCloseReason::Shutdown:
        printw("Reason: server is going down");
        break;
    default:
        printw("Reason: unknown {}", (int)reason);
    }

    delete data;
    socket->setUserData(nullptr);
}

void onReadyRead(size_t index, appfw::TcpClientSocket4Ptr socket) noexcept {
    std::vector<uint8_t> buf(2048);
    int size = 0;

    try {
        size = socket->read(buf);
    } catch (const appfw::NetworkErrorException &e) {
        printe("Failed to read message from client {}: {}",
               socket->getRemoteAddress().toString() , e.what());
        return;
    }

    if (size == 0) {
        // EOF
        socket->close(appfw::SocketCloseReason::ConnAborted);
        return;
    }

    auto *data = static_cast<ClientData *>(socket->getUserData());
    printn("{} [{}] [size = {}]", socket->getRemoteAddress().toString(), data->id, size);
    std::string_view str = std::string_view((char *)buf.data(), size);
    printi("{}", str);

    try {
        // Echo back
        std::string echo = "Echo: " + std::string(str);
        socket->writeAll(appfw::span((uint8_t *)echo.data(), echo.size()));
    } catch (const appfw::NetworkErrorException &e) {
        printe("Failed to send back response: {}", e.what());
    }
}

int main(int argc, char **argv) {
    appfw::InitComponent appfwInit(appfw::InitOptions().setArgs(argc, argv));
    printn("TCP Server Example");

    if (getCommandLine().isFlagSet("--help")) {
        printi("Usage: {} [--port port]", getCommandLine().getCommandName());
        return -1;
    }

    g_Server.setConnAcceptedCallback(onConnAccepted);
    g_Server.setConnClosedCallback(onConnClosed);
    g_Server.setReadyReadCallback(onReadyRead);
    g_Server.startListening(appfw::ADDR4_ANY, getCommandLine().getArgInt("--port", 27015));
    printi("Big Brother is listening at {}", g_Server.getListenAddress().toString());

    printi("Type 'clients' for the list of connected clients");
    printi("Type 'send <id> <msg>' to send a message");
    printi("Type 'kick <id>' to disconnect a client");
    printi("Type 'quit' to exit");

    getCommandLine().execCommands();

    while (run_app.getValue()) {
        appfw::mainLoopTick();

        try {
            g_Server.poll(10); // wait for 10 ms for a connection or data
        } catch (const appfw::NetworkErrorException &e) {
            printfatal("Fatal Error: {}", e.what());
            return -1;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((1000 / 60) - 10));
    }

    g_Server.sendToAll(appfw::span((uint8_t *)"Gonna Give You Up\n", 18));
    g_Server.sendToAll(appfw::span((uint8_t *)"Gonna Let You Down\n", 19));
    g_Server.stopListening();

    printn("Goodbye!");
}
