#include <algorithm>
#include <stdexcept>
#include <appfw/binary_buffer.h>

appfw::BinaryBuffer::BinaryBuffer(appfw::span<uint8_t> buffer) {
    m_Buf = buffer;
}

void appfw::BinaryBuffer::readBytes(uint8_t *buf, size_t size) {
    if (m_iOffset + size > m_Buf.size()) {
        throw std::out_of_range("no data left");
    }

    memcpy(buf, m_Buf.data() + m_iOffset, size);
    m_iOffset += size;
}

void appfw::BinaryBuffer::writeBytes(const uint8_t *buf, size_t size) {
    if (m_iOffset + size > m_Buf.size()) {
        throw std::out_of_range("no space left");
    }

    memcpy(m_Buf.data() + m_iOffset, buf, size);
    m_iOffset += size;
}

appfw::binpos appfw::BinaryBuffer::bytesLeftToRead() const {
    return m_Buf.size() - m_iOffset;
}

appfw::binpos appfw::BinaryBuffer::bytesLeftToWrite() const {
    return m_Buf.size() - m_iOffset;
}

appfw::binpos appfw::BinaryBuffer::getPosition() const {
    return m_iOffset;
}

void appfw::BinaryBuffer::seekRelative(binpos offset) {
    m_iOffset = std::clamp((binpos)(m_iOffset + offset), (binpos)0, (binpos)m_Buf.size());
}

void appfw::BinaryBuffer::seekAbsolute(binpos offset) {
    m_iOffset = std::clamp(offset, (binpos)0, (binpos)m_Buf.size());
}
