#include <appfw/binary_stream.h>
#include <appfw/binary_buffer.h>
#include <doctest/doctest.h>

namespace {

struct TestData {
    char c = 'D';
    uint8_t b = 87;
    int8_t sb = -27;
    uint16_t ui16 = 28974;
    int16_t i16 = -28974;
    uint32_t ui32 = 2115479984;
    int32_t i32 = -2115479984;
    uint64_t ui64 = 21154799845465435;
    int64_t i64 = -21154799845465435;
};

static_assert(std::is_trivially_copyable<TestData>::value, "TestData is not trivial");

}

TEST_CASE("Binary Streams") {
    constexpr size_t BUF_SIZE = 1000;
    std::vector<uint8_t> databuf(BUF_SIZE);
    appfw::BinaryBuffer stream(databuf);

    TestData testData;
    TestData testDataArray[10];
    const std::string STR1 = "Test string!";
    const std::string STR2 = "";
    stream.writeChar(testData.c);
    stream.writeByte(testData.b);
    stream.writeSByte(testData.sb);
    stream.writeUInt16(testData.ui16);
    stream.writeInt16(testData.i16);
    stream.writeUInt32(testData.ui32);
    stream.writeInt32(testData.i32);
    stream.writeUInt64(testData.ui64);
    stream.writeInt64(testData.i64);
    stream.writeString(STR1);
    stream.writeString(STR2);
    stream.writeObject(testData);
    stream.writeObjectArray(appfw::span(testDataArray).const_span());
    stream.seekAbsolute(0);
    
    std::string str;
    CHECK(stream.readChar() == testData.c);
    CHECK(stream.readByte() == testData.b);
    CHECK(stream.readSByte() == testData.sb);
    CHECK(stream.readUInt16() == testData.ui16);
    CHECK(stream.readInt16() == testData.i16);
    CHECK(stream.readUInt32() == testData.ui32);
    CHECK(stream.readInt32() == testData.i32);
    CHECK(stream.readUInt64() == testData.ui64);
    CHECK(stream.readInt64() == testData.i64);
    CHECK(stream.readString() == STR1);
    CHECK(stream.readString() == STR2);

    TestData testDataRead;
    stream.readObject(testDataRead);
    CHECK(memcmp(&testData, &testDataRead, sizeof(testData)) == 0);

    TestData testDataArrayRead[10];
    stream.readObjectArray(appfw::span(testDataArrayRead));
    CHECK(memcmp(&testDataArray, &testDataArrayRead, sizeof(testDataArray)) == 0);
}
