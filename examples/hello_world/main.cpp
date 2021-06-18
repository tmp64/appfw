#include <thread>
#include <chrono>
#include <appfw/appfw.h>
#include <appfw/init.h>

ConVar<bool> run_app("run_app", true, "Whether or not the app should be running");

ConCommand cmd_quit("quit", "Quits the app", []() { run_app.setValue(false); });

int main(int argc, char **argv) {
    appfw::InitComponent appfwInit(appfw::InitOptions().setArgs(argc, argv));
    printn("Hello, world!");
    printi("Type 'list' for the list of commands");
    printi("Type 'quit' to exit");

    ConVar<std::string> rt_cvar("rt_cvar", "yes", "Cvar created after init");

    while (run_app.getValue()) {
        appfw::mainLoopTick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    printn("Goodbye!");
}
