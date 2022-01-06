#ifndef APPFW_NETWORK_DATAGRAM_PARSER_H
#define APPFW_NETWORK_DATAGRAM_PARSER_H
#include <vector>
#include <functional>
#include <appfw/binary_stream.h>

namespace appfw {

//! Simple datagram parser for stream protocols (like TCP).
//! Packet structure:
//! - Header
//!   - Magic (up to MAX_MAGIC_SIZE bytes)
//!   - Payload size (uint32_t, little-endian)
//! - Payload (of payload size)
class DatagramParser {
public:
    static constexpr uint16_t MAX_MAGIC_SIZE = 30;
    static constexpr uint32_t MAX_ABSOLUTE_PAYLOAD_SIZE = std::numeric_limits<uint32_t>::max() - MAX_MAGIC_SIZE - sizeof(uint32_t);

    enum class Error {
        NoError,
        InvalidMagic,
        PayloadTooLarge,
    };

    //! Called when full payload is received.
    //! It is NOT recommended to modify 'payload' if you use 'stream'.
    //! @param  stream  Binary stream of the payload
    //! @param  payload Payload buffer (same as stream).
    //! @param  size    Payload size
    using PayloadCallback = std::function<void(appfw::BinaryInputStream &stream, uint8_t *payload, size_t size)>;

    //! Sets the magic bytes.
    void setMagic(const uint8_t *magic, uint16_t size);

    //! Sets the magic bytes.
    void setMagic(const char *magic, uint16_t size);

    //! Sets the maximum payload size.
    void setMaxPayloadSize(uint32_t size);

    //! Sets the payload callback.
    void setPayloadCallback(const PayloadCallback &cb);

    //! Resets the internal state.
    void reset();

    //! Parses incoming stream of data. When payload is fully read, calls payload callback.
    //! If an error occures, sets error and returns false. State is undefined, it's best to close the connection and call reset().
    //! @param  data        Incoming data
    //! @param  size        Size of the data
    //! @param  bytesRead   Number of bytes read by the call
    //! @param  error       Error that was encountered
    //! @returns success or not
    bool parseData(const uint8_t *data, uint32_t size, uint32_t &bytesRead, Error &error);

    //! @returns the text of an error
    static std::string_view getErrorString(Error error);

private:
    static_assert(sizeof(uint32_t) == 4, "You're using a weird system.");

    enum class Stage : uint8_t {
        Magic,
        Size,
        Payload
    };

    uint8_t m_Magic[MAX_MAGIC_SIZE];
    uint16_t m_uMagicSize = 0;
    uint32_t m_uMaxPayloadSize = 0;
    PayloadCallback m_fnPayloadCallback;

    Stage m_Stage;
    uint32_t m_uParsedPacketSize;
    uint8_t m_ParsedMagic[MAX_MAGIC_SIZE];
    uint8_t m_ParsedPayloadSizeBytes[sizeof(uint32_t)];
    uint32_t m_uParsedPayloadSize;
    std::vector<uint8_t> m_ParsedPayload;

    //! @returns the size of the header.
    inline uint32_t getHeaderSize() { return m_uMagicSize + sizeof(uint32_t); }
};

}

#endif
