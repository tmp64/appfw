#include <appfw/binary_file.h>

void appfw::BinaryInputFile::open(const fs::path &path) {
    std::ifstream file(path);
    open(std::move(file));
}

void appfw::BinaryInputFile::open(std::ifstream &&file) {
    m_File = std::move(file);
    m_File.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    m_iFileSize = getFileSize(m_File);
}

void appfw::BinaryInputFile::readBytes(uint8_t *buf, size_t size) {
    m_File.read(reinterpret_cast<char *>(buf), size);
}

appfw::binpos appfw::BinaryInputFile::bytesLeftToRead() const {
    if (m_File.eof()) {
        return 0;
    } else {
        return m_iFileSize - getPosition();
    }
}

appfw::binpos appfw::BinaryInputFile::getPosition() const {
    return const_cast<std::ifstream &>(m_File).tellg();
}

void appfw::BinaryInputFile::seekRelative(binpos offset) {
    m_File.seekg(offset, std::ios::cur);
}

void appfw::BinaryInputFile::seekAbsolute(binpos offset) {
    if (offset == STREAM_SEEK_END) {
        m_File.seekg(offset, std::ios::end);
    } else {
        m_File.seekg(offset);
    }
}

void appfw::BinaryOutputFile::open(const fs::path &path) {
    std::ofstream file(path);
    open(std::move(file));
}

void appfw::BinaryOutputFile::open(std::ofstream &&file) {
    m_File = std::move(file);
    m_File.exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

void appfw::BinaryOutputFile::writeBytes(const uint8_t *buf, size_t size) {
    m_File.write(reinterpret_cast<const char *>(buf), size);
}

appfw::binpos appfw::BinaryOutputFile::bytesLeftToWrite() const {
    // Unknown
    return std::numeric_limits<binpos>::max();
}

appfw::binpos appfw::BinaryOutputFile::getPosition() const {
    return const_cast<std::ofstream &>(m_File).tellp();
}

void appfw::BinaryOutputFile::seekRelative(binpos offset) {
    m_File.seekp(offset, std::ios::cur);
}

void appfw::BinaryOutputFile::seekAbsolute(binpos offset) {
    if (offset == STREAM_SEEK_END) {
        m_File.seekp(offset, std::ios::end);
    } else {
        m_File.seekp(offset);
    }
}
