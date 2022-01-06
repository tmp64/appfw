#include <cstring>
#include <appfw/binary_buffer.h>
#include <appfw/dbg.h>
#include <appfw/network/datagram_parser.h>

void appfw::DatagramParser::setMagic(const uint8_t *magic, uint16_t size) {
    AFW_ASSERT(size > 0 && size <= MAX_MAGIC_SIZE);
    std::memcpy(m_Magic, magic, size);
    m_uMagicSize = size;
}

void appfw::DatagramParser::setMagic(const char *magic, uint16_t size) {
    setMagic(reinterpret_cast<const uint8_t *>(magic), size);
}

void appfw::DatagramParser::setMaxPayloadSize(uint32_t size) {
    AFW_ASSERT(size <= MAX_ABSOLUTE_PAYLOAD_SIZE);
    m_uMaxPayloadSize = size;
    m_ParsedPayload.resize(size);
}

void appfw::DatagramParser::setPayloadCallback(const PayloadCallback &cb) {
    m_fnPayloadCallback = cb;
}

void appfw::DatagramParser::reset() {
    m_Stage = Stage::Magic;
    m_uParsedPacketSize = 0;
    m_uParsedPayloadSize = 0;
}

bool appfw::DatagramParser::parseData(const uint8_t *data, uint32_t size, uint32_t &outBytesRead,
                                      Error &error) {
    AFW_ASSERT(m_uMagicSize > 0);
    error = Error::NoError;
    uint32_t bytesRead = 0;

    while (bytesRead != size) {
        uint32_t readSize = 0; //!< Number of bytes read this iteration

        switch (m_Stage) {
        case Stage::Magic: {
            AFW_ASSERT(m_uParsedPacketSize < m_uMagicSize);
            uint32_t pos = m_uParsedPacketSize;
            uint32_t leftToRead = m_uMagicSize - pos;
            readSize = std::min(leftToRead, size);
            std::memcpy(m_ParsedMagic + pos, data + bytesRead, readSize);
            m_uParsedPacketSize += readSize;

            if (readSize == leftToRead) {
                // Full magic is read
                if (std::memcmp(m_Magic, m_ParsedMagic, m_uMagicSize)) {
                    error = Error::InvalidMagic;
                    return false;
                }

                m_Stage = Stage::Size;
            }
            break;
        }
        case Stage::Size: {
            AFW_ASSERT(m_uParsedPacketSize >= m_uMagicSize &&
                       m_uParsedPacketSize < m_uMagicSize + sizeof(uint32_t));
            uint32_t pos = m_uParsedPacketSize - m_uMagicSize;
            uint32_t leftToRead = sizeof(uint32_t) - pos;
            readSize = std::min(leftToRead, size);
            std::memcpy(m_ParsedPayloadSizeBytes + pos, data + bytesRead, readSize);
            m_uParsedPacketSize += readSize;

            if (readSize == leftToRead) {
                uint32_t rawPayloadSize;
                std::memcpy(&rawPayloadSize, m_ParsedPayloadSizeBytes, sizeof(uint32_t));
                m_uParsedPayloadSize = appfw::littleEndianSwap(rawPayloadSize);

                if (m_uParsedPacketSize > m_uMaxPayloadSize) {
                    error = Error::PayloadTooLarge;
                    return false;
                }

                m_Stage = Stage::Payload;
            }
            break;
        }
        case Stage::Payload: {
            uint32_t pos = m_uParsedPacketSize - getHeaderSize();
            uint32_t leftToRead = m_uParsedPayloadSize - pos;
            readSize = std::min(leftToRead, size);
            std::memcpy(m_ParsedPayload.data() + pos, data + bytesRead, readSize);
            m_uParsedPacketSize += readSize;

            if (readSize == leftToRead) {
                appfw::span<uint8_t> payload(m_ParsedPayload.data(), m_uParsedPayloadSize);
                appfw::BinaryBuffer binBuf(payload);
                m_fnPayloadCallback(binBuf, payload.data(), payload.size());
                reset();
            }
            break;
        }
        }

        bytesRead += readSize;
    }

    outBytesRead = bytesRead;
    return true;
}

std::string_view appfw::DatagramParser::getErrorString(Error error) {
    switch (error) {
    case Error::NoError: return "No error";
    case Error::InvalidMagic: return "Invalid magic";
    case Error::PayloadTooLarge: return "Payload too large";
    default: return "Unknown";
    }
}
