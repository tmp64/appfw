#ifndef APPFW_BINARY_BUFFER_H
#define APPFW_BINARY_BUFFER_H
#include <appfw/binary_stream.h>
#include <appfw/span.h>

namespace appfw {

/**
 * Class that implements binary streams using a preallocated byte array.
 */
class BinaryBuffer : public BinaryInputStream, public BinaryOutputStream {
public:
    BinaryBuffer() = default;
    BinaryBuffer(appfw::span<uint8_t> buffer);
    void readBytes(uint8_t *buf, size_t size) override;
    void writeBytes(const uint8_t *buf, size_t size) override;
    binpos bytesLeftToRead() const override;
    binpos bytesLeftToWrite() const override;
    binpos getPosition() const override;
    void seekRelative(binpos offset) override;
    void seekAbsolute(binpos offset) override;

    inline appfw::span<uint8_t> getBuffer() { return m_Buf; }

private:
    appfw::span<uint8_t> m_Buf;
    size_t m_iOffset = 0;
};

}

#endif
