#ifndef APPFW_EXTCON_OPCODES_H
#define APPFW_EXTCON_OPCODES_H
#include <cstdint>

namespace appfw {

//! Maximum size of an extcon message payload
static constexpr uint32_t EXTCON_MAX_PAYLOAD_SIZE = 16 * 1024;

//! Extcon message magic
static constexpr uint8_t EXTCON_MSG_MAGIC[] = "ECON01";
static constexpr uint16_t EXTCON_MSG_MAGIC_SIZE = 6;

//! Extcon opcodes
enum ExtconOpcodes : uint8_t {
    // Client -> Server
    EXTCON_OPCODE_COMMAND = 1,

    // Client <-> Server
    EXTCON_OPCODE_REQUEST_FOCUS = 51,

    // Server -> Client
    EXTCON_OPCODE_PRINT = 101,
    EXTCON_OPCODE_CMD_LIST = 102,
};

} // namespace appfw

#endif
