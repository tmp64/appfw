#ifndef APPFW_BINARY_STREAM_H
#define APPFW_BINARY_STREAM_H
#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>
#include <appfw/span.h>

namespace appfw {

/**
 * Numeric type for position in a stream.
 */
using binpos = int64_t;

/**
 * Use this in seekAbsolute to seek to the end
 */
constexpr binpos STREAM_SEEK_END = std::numeric_limits<binpos>::max();

/**
 * Interface for a stream of binary data that can be read from.
 * All data is stored packed in little-endian format and converted automatically.
 */
class BinaryInputStream {
public:
    virtual ~BinaryInputStream() = default;

    /**
     * Reads `size` bytes from the stream into a buffer.
     * On error throws `std::out_of_range`.
     */
    virtual void readBytes(uint8_t *buf, size_t size) = 0;

    /**
     * Returns number of bytes that can be read from the stream.
     */
    virtual binpos bytesLeftToRead() const = 0;

    /**
     * How many bytes were read from the stream.
     */
    virtual binpos getPosition() const = 0;

    /**
     * Seeks N bytes forward or backward relative to current position.
     */
    virtual void seekRelative(binpos offset) = 0;

    /**
     * Seeks to a specified position in the stream.
     */
    virtual void seekAbsolute(binpos offset) = 0;

    char readChar();
    uint8_t readByte();
    int8_t readSByte();

    uint16_t readUInt16();
    int16_t readInt16();

    uint32_t readUInt32();
    int32_t readInt32();

    uint64_t readUInt64();
    int64_t readInt64();

    float readFloat();
    double readDouble();

    void readString(std::string &str);
    std::string readString();

    /**
     * Reads a written object
     */
    template <typename T>
    void readObject(const T &obj) {
        static_assert(sizeof(obj) <= std::numeric_limits<uint32_t>::max(),
                      "Object size is too large");
        uint32_t objSize = readUInt32();
        
        if (objSize != sizeof(obj)) {
            throw std::runtime_error("object size is different");
        }

        readBytes(reinterpret_cast<uint8_t *>(&obj), sizeof(obj));
    }

    /**
     * Returns the size of written object array without changing position.
     */
    template <typename T>
    size_t peekObjectArraySize() {
        static_assert(sizeof(T) <= std::numeric_limits<uint32_t>::max(),
                      "Object size is too large");
        uint32_t objSize = readUInt32();
        uint64_t arraySize = readUInt64();
        seekRelative(-(sizeof(objSize) + sizeof(arraySize)));

        if (objSize != sizeof(obj)) {
            throw std::runtime_error("object size is different");
        }

        if (arraySize > (uint64_t)std::numeric_limits<size_t>::max()) {
            throw std::runtime_error("array is too large");
        }

        return arraySize;
    }

    /**
     * Reads a written object array.
     */
    template <typename T>
    void readObjectArray(appfw::span<T> data) {
        static_assert(sizeof(T) <= std::numeric_limits<uint32_t>::max(),
                      "Object size is too large");
        
        uint32_t objSize = readUInt32();
        uint64_t arraySize = readUInt64();

        if (objSize != sizeof(obj)) {
            throw std::runtime_error("object size is different");
        }

        if (arraySize > (uint64_t)data.size()) {
            throw std::runtime_error("input array is not large enough");
        }

        readBytes(reinterpret_cast<uint8_t *>(data.data()), sizeof(T) * arraySize);
    }
};

/**
 * Interface for a stream of binary data that can be written to.
 * All data is stored packed in little-endian format and converted automatically.
 */
class BinaryOutputStream {
public:
    virtual ~BinaryOutputStream() = default;

    /**
     * Writes `size` bytes into the stream from a buffer.
     * On error throws `std::out_of_range`.
     */
    virtual void writeBytes(const uint8_t *buf, size_t size) = 0;

    /**
     * Returns number of bytes that can be writeen into the stream.
     */
    virtual binpos bytesLeftToWrite() const = 0;

    /**
     * How many bytes were written into the stream.
     */
    virtual binpos getPosition() const = 0;

    /**
     * Seeks N bytes forward or backward relative to current position.
     */
    virtual void seekRelative(binpos offset) = 0;

    /**
     * Seeks to a specified position in the stream.
     */
    virtual void seekAbsolute(binpos offset) = 0;

    void writeChar(const char val);
    void writeByte(const uint8_t val);
    void writeSByte(const int8_t val);

    void writeUInt16(const uint16_t val);
    void writeInt16(const int16_t val);

    void writeUInt32(const uint32_t val);
    void writeInt32(const int32_t val);

    void writeUInt64(const uint64_t val);
    void writeInt64(const int64_t val);

    void writeFloat(const float val);
    void writeDouble(const double val);

    void writeString(std::string_view str);

    /**
     * Reinterprets the object as bytes and writes the bytes into the buffer.
     */
    template <typename T>
    void writeObject(const T &obj) {
        static_assert(sizeof(obj) <= std::numeric_limits<uint32_t>::max(),
                      "Object size is too large");
        writeUInt32(sizeof(obj));
        writeBytes(reinterpret_cast<uint8_t *>(&obj), sizeof(obj));
    }

    /**
     * Reinterprets the objects as bytes and writes the bytes into the buffer.
     */
    template <typename T>
    void writeObjectArray(appfw::span<T> data) {
        static_assert(sizeof(T) <= std::numeric_limits<uint32_t>::max(),
                      "Object size is too large");
        writeUInt32(sizeof(T));
        writeUInt64(data.size());
        writeBytes(reinterpret_cast<uint8_t *>(data.data()), sizeof(T) * data.size());
    }
};

}

#endif
