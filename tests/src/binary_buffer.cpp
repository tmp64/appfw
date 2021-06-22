#include <cstring>
#include <appfw/binary_buffer.h>
#include <doctest/doctest.h>

TEST_CASE("appfw::BinaryBuffer") {
    constexpr size_t BUF_SIZE = 1000;
    std::vector<uint8_t> databuf(BUF_SIZE);
    appfw::BinaryBuffer stream(databuf);

    CHECK(stream.bytesLeftToRead() == BUF_SIZE);
    CHECK(stream.bytesLeftToWrite() == BUF_SIZE);
    CHECK(stream.getPosition() == 0);
    stream.seekRelative(-1);
    CHECK(stream.getPosition() == 0);
    stream.seekRelative(1);
    CHECK(stream.getPosition() == 1);
    stream.seekAbsolute(0);
    CHECK(stream.getPosition() == 0);
    stream.seekAbsolute(10 * BUF_SIZE); // can't seek past the buffer
    CHECK(stream.getPosition() == BUF_SIZE);

    // Check writing
    stream.seekAbsolute(0);
    constexpr uint8_t TEST_DATA[] = {1, 2, 3, 4, 5, 6, 7, 8};
    stream.writeBytes(TEST_DATA, std::size(TEST_DATA));
    CHECK(stream.getPosition() == std::size(TEST_DATA));

    // Check reading
    stream.seekAbsolute(0);
    uint8_t readData[std::size(TEST_DATA)] = {};
    stream.readBytes(readData, std::size(TEST_DATA));
    CHECK(std::memcmp(TEST_DATA, readData, std::size(TEST_DATA)) == 0);

    // Seek to almost the end
    stream.seekAbsolute(appfw::STREAM_SEEK_END);
    stream.seekRelative(-3);
    CHECK_THROWS(stream.readBytes(readData, std::size(TEST_DATA)));
}
