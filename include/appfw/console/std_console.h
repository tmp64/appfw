#ifndef APPFW_CONSOLE_STD_CONSOLE_H
#define APPFW_CONSOLE_STD_CONSOLE_H
#include <appfw/console/console_system.h>
#include <appfw/console/term_console.h>

namespace appfw {

/**
 * Terminal console using std::cin and std::cout.
 */
class StdConsole : public ITermConsole, NoMove {
public:
    StdConsole(TermInputMethod inputMethod);
    ~StdConsole();

    void onAdd(ConsoleSystem *conSys) override;
    void onRemove(ConsoleSystem *conSys) override;
    void print(const ConMsgInfo &info, std::string_view msg) override;
    bool isThreadSafe() override;

    TermInputMethod getInputMethod() override;
    void tick() override;

private:
    struct SyncData {
        bool bIsValid = false;
        std::mutex mutex;
    };

    ConsoleSystem *m_pConSys = nullptr;
    std::shared_ptr<SyncData> m_pSyncData;
    std::thread m_Thread;
    std::queue<std::string> m_CmdQueue;
    TermInputMethod m_InputMethod;

    void setColor(ConMsgColor color);

    static void threadWorker(StdConsole *t, std::shared_ptr<SyncData> pSyncData);
};

} // namespace appfw

#endif
