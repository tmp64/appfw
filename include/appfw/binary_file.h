#ifndef APPFW_BINARY_FILE_H
#define APPFW_BINARY_FILE_H
#include <fstream>
#include <appfw/binary_stream.h>
#include <appfw/filesystem.h>

namespace appfw {

class BinaryInputFile : public BinaryInputStream {
public:
    BinaryInputFile() = default;

    inline BinaryInputFile(const fs::path &path) { open(path); }

    /**
     * Opens a file.
     */
    void open(const fs::path &path);

    /**
     * Moves an open file into the stream.
     * The file will be seeked to the beginning.
     */
    void open(std::ifstream &&file);

    void readBytes(uint8_t *buf, size_t size) override;
    binpos bytesLeftToRead() const override;
    binpos getPosition() const override;
    void seekRelative(binpos offset) override;
    void seekAbsolute(binpos offset) override;

private:
    std::ifstream m_File;
    binpos m_iFileSize = 0;
};

class BinaryOutputFile : public BinaryOutputStream {
public:
    BinaryOutputFile() = default;

    inline BinaryOutputFile(const fs::path &path) { open(path); }

    /**
     * Opens a file.
     */
    void open(const fs::path &path);

    /**
     * Moves an open file into the stream.
     */
    void open(std::ofstream &&file);

    void writeBytes(const uint8_t *buf, size_t size) override;
    binpos bytesLeftToWrite() const override;
    binpos getPosition() const override;
    void seekRelative(binpos offset) override;
    void seekAbsolute(binpos offset) override;

private:
    std::ofstream m_File;
};

} // namespace appfw

#endif
