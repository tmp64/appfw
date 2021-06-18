#include <cassert>
#include <appfw/cmd_buffer.h>
#include <appfw/dbg.h>

appfw::CmdBuffer::CmdBuffer(const CommandHandler &handler) {
    setCommandHandler(handler);
}

void appfw::CmdBuffer::append(const std::string &cmd) {
    append(CmdString::parse(cmd));
}

void appfw::CmdBuffer::append(const CmdString &cmd) {
    std::lock_guard lock(m_Mutex);
    if (!cmd.empty()) {
        m_CmdQueue.push(cmd);
    }
}

void appfw::CmdBuffer::append(const std::vector<CmdString> &cmds) {
    std::lock_guard lock(m_Mutex);
    for (auto &i : cmds) {
        if (!i.empty()) {
            m_CmdQueue.push(i);
        }
    }
}

void appfw::CmdBuffer::executeOnce() {
    AFW_ASSERT_MSG(m_Handler, "Command buffer has no handler");
    std::lock_guard lock(m_Mutex);

    if (!m_CmdQueue.empty()) {
        CmdString &cmd = m_CmdQueue.front();
        m_Handler(cmd);
        m_CmdQueue.pop();
    }
}

void appfw::CmdBuffer::executeAll() {
    AFW_ASSERT_MSG(m_Handler, "Command buffer has no handler");
    std::lock_guard lock(m_Mutex);
    
    size_t count = m_CmdQueue.size();
    for (size_t i = 0; i < count; i++) {
        CmdString &cmd = m_CmdQueue.front();
        m_Handler(cmd);
        m_CmdQueue.pop();
    }
}

size_t appfw::CmdBuffer::getCommandCount() { return m_CmdQueue.size(); }

void appfw::CmdBuffer::setCommandHandler(const CommandHandler &handler) {
    m_Handler = handler;
}
