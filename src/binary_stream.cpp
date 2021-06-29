#include <cstring>
#include <limits>
#include <stdexcept>
#include <appfw/binary_stream.h>
#include <appfw/platform.h>

void appfw::BinaryInputStream::readByteSpan(appfw::span<uint8_t> data) {
    readBytes(data.data(), data.size());
}

char appfw::BinaryInputStream::readChar() {
    char val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return val;
}

uint8_t appfw::BinaryInputStream::readByte() {
    uint8_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return val;
}

int8_t appfw::BinaryInputStream::readSByte() {
    int8_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return val;
}

uint16_t appfw::BinaryInputStream::readUInt16() {
    uint16_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

int16_t appfw::BinaryInputStream::readInt16() {
    int16_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

uint32_t appfw::BinaryInputStream::readUInt32() {
    uint32_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

int32_t appfw::BinaryInputStream::readInt32() {
    int32_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

uint64_t appfw::BinaryInputStream::readUInt64() {
    uint64_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

int64_t appfw::BinaryInputStream::readInt64() {
    int64_t val;
    readBytes(reinterpret_cast<uint8_t *>(&val), sizeof(val));
    return appfw::littleEndianSwap(val);
}

float appfw::BinaryInputStream::readFloat() {
    static_assert(sizeof(float) == 4, "float is of incorrect size");
    static_assert(std::numeric_limits<float>::is_iec559, "float is not IEEE-754");
    float val;
    uint32_t bits;
    bits = readUInt32();
    memcpy(&val, &bits, sizeof(bits));
    return val;
}

double appfw::BinaryInputStream::readDouble() {
    static_assert(sizeof(double) == 8, "double is of incorrect size");
    static_assert(std::numeric_limits<double>::is_iec559, "double is not IEEE-754");
    double val;
    uint64_t bits;
    bits = readUInt64();
    memcpy(&val, &bits, sizeof(bits));
    return val;
}

void appfw::BinaryInputStream::readString(std::string &str) {
    uint32_t len = readUInt32();
    str.resize(len);
    readBytes((uint8_t *)str.data(), len);
}

std::string appfw::BinaryInputStream::readString() {
    std::string str;
    readString(str);
    return str;
}

void appfw::BinaryOutputStream::writeByteSpan(appfw::span<const uint8_t> data) {
    writeBytes(data.data(), data.size());
}

void appfw::BinaryOutputStream::writeByteSpan(appfw::span<uint8_t> data) {
    writeByteSpan(data.const_span());
}

void appfw::BinaryOutputStream::writeChar(const char val) {
    static_assert(sizeof(val) == 1, "char is of incorrect size");
    writeBytes(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
}

void appfw::BinaryOutputStream::writeByte(const uint8_t val) {
    static_assert(sizeof(val) == 1, "uint8_t is of incorrect size");
    writeBytes(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
}

void appfw::BinaryOutputStream::writeSByte(const int8_t val) {
    static_assert(sizeof(val) == 1, "int8_t is of incorrect size");
    writeBytes(reinterpret_cast<const uint8_t *>(&val), sizeof(val));
}

void appfw::BinaryOutputStream::writeUInt16(const uint16_t val) {
    static_assert(sizeof(val) == 2, "uint16_t is of incorrect size");
    uint16_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeInt16(const int16_t val) {
    static_assert(sizeof(val) == 2, "int16_t is of incorrect size");
    int16_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeUInt32(const uint32_t val) {
    static_assert(sizeof(val) == 4, "uint32_t is of incorrect size");
    uint32_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeInt32(const int32_t val) {
    static_assert(sizeof(val) == 4, "int32_t is of incorrect size");
    int32_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeUInt64(const uint64_t val) {
    static_assert(sizeof(val) == 8, "uint64_t is of incorrect size");
    uint64_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeInt64(const int64_t val) {
    static_assert(sizeof(val) == 8, "int64_t is of incorrect size");
    int64_t val2 = appfw::littleEndianSwap(val);
    writeBytes(reinterpret_cast<const uint8_t *>(&val2), sizeof(val2));
}

void appfw::BinaryOutputStream::writeFloat(const float val) {
    static_assert(sizeof(val) == 4, "float is of incorrect size");
    static_assert(std::numeric_limits<float>::is_iec559, "float is not IEEE-754");
    uint32_t bits;
    memcpy(&bits, &val, sizeof(bits));
    writeUInt32(bits);
}

void appfw::BinaryOutputStream::writeDouble(const double val) {
    static_assert(sizeof(val) == 8, "double is of incorrect size");
    static_assert(std::numeric_limits<double>::is_iec559, "double is not IEEE-754");
    uint64_t bits;
    memcpy(&bits, &val, sizeof(bits));
    writeUInt64(bits);
}

void appfw::BinaryOutputStream::writeString(std::string_view str) {
    if (str.size() >= std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range("string is too large");
    }

    writeUInt32((uint32_t)str.size());
    writeBytes((const uint8_t *)str.data(), str.size());
}
