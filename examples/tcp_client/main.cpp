#include <thread>
#include <chrono>
#include <appfw/appfw.h>
#include <appfw/init.h>
#include <appfw/network/tcp_client4.h>

appfw::TcpClient4 g_Client;
std::string g_LastHost;
std::string g_LastPort;

ConVar<bool> run_app("run_app", true, "Whether or not the app should be running");
ConVar<int> timeout("timeout", 5000, "Connection timeout in ms");
ConCommand cmd_quit("quit", "Quits the app", []() { run_app.setValue(false); });

ConCommand cmd_connect("connect", "Connects to a host", [](const CmdString &args) {
    if (args.size() != 3) {
        printi("Usage: connect <host> <port>");
        return;
    }

    if (g_Client.getStatus() != appfw::NetClientStatus::Closed) {
        printe("Connection is already open");
        return;
    }

    try {
        g_LastHost = args[1];
        g_LastPort = args[2];

        printi("Resolving {}:{}...", args[1], args[2]);
        auto list = appfw::resolveHostName(args[1], args[2]);

        if (list.empty()) {
            printe("Failed to resolve the hostname");
            return;
        }

        appfw::SockAddr4 addr = *list.begin();
        printi("Connecting to {}", addr.toString());
        g_Client.connect(addr, timeout.getValue());

        if (g_Client.getStatus() == appfw::NetClientStatus::Connected) {
            printn("Connection established");
        }
    } catch (const std::exception &e) {
        printe("Connection failed: {}", e.what());
    }
});

ConCommand cmd_disconnect("disconnect", "Disconnects current connection", []() { g_Client.close(); });

ConCommand cmd_retry("retry", "Retries last connection", []() {
    if (g_Client.getStatus() != appfw::NetClientStatus::Closed) {
        g_Client.close();
    }

    appfw::getConsole().commandNow(fmt::format("connect \"{}\" \"{}\"", g_LastHost, g_LastPort));
});

ConCommand cmd_send("send", "Sends a message to a client", [](const CmdString &args) {
    if (args.size() != 2) {
        printi("Usage: send \"<msg>\"");
        return;
    }

    if (g_Client.getStatus() != appfw::NetClientStatus::Connected) {
        printe("Not connected.");
        return;
    }

    try {
        g_Client.writeAll(appfw::span((uint8_t *)args[1].data(), args[1].size()));
        g_Client.writeAll(appfw::span((uint8_t *)"\n", 1));
        printi("Message sent");
    } catch (const appfw::NetworkErrorException &e) {
        printe("Send failed: {}", e.what());
    }
});

void onReadyRead() {
    std::vector<uint8_t> buf(2048);
    int size = 0;
    try {
        size = g_Client.read(buf);
    } catch (const appfw::NetworkErrorException &e) {
        printe("Failed to read message: {}", e.what());
        return;
    }

    if (size == 0) {
        // EOF
        g_Client.close();
        printn("Connection closed from the other side");
        return;
    }

    printn("Data received [size = {}]", size);
    printi("{}", std::string_view((char *)buf.data(), size));
}

int main(int argc, char **argv) {
    appfw::InitComponent appfwInit(appfw::InitOptions().setArgs(argc, argv));
    printn("TCP Client Example");
    printi("Type 'connect <host> <port>' to connect");
    printi("Type 'disconnect' to disconnect");
    printi("Type 'retry' to reconnect to last host");
    printi("Type 'send <msg>' to send a message");
    printi("Type 'quit' to exit");

    getCommandLine().execCommands();

    while (run_app.getValue()) {
        appfw::mainLoopTick();
        
        try {
            if (g_Client.getStatus() == appfw::NetClientStatus::Connecting) {
                g_Client.updateStatus(10);

                if (g_Client.getStatus() == appfw::NetClientStatus::Connected) {
                    printn("Connection established");
                }
            } else if (g_Client.getStatus() == appfw::NetClientStatus::Connected) {
                if (g_Client.updateStatus(10)) {
                    // New data
                    onReadyRead();
                }
            }
        } catch (const std::exception &e) {
            printe("Error: {}", e.what());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds((1000 / 60) - 10));
    }

    g_Client.close();
    printn("Goodbye!");
}
