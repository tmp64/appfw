#include <cstdint>
#include <cstring>
#include <appfw/platform.h>
#include <doctest/doctest.h>

TEST_CASE("Endianness detection") {
	// Validate endianness detection
    static_assert(sizeof(uint16_t) == 2, "uint16_t is not two bytes");
    uint16_t end = 0xDEAD;
    uint8_t endBytes[sizeof(end)];
    std::memcpy(endBytes, &end, sizeof(end));

    if (appfw::isLittleEndian()) {
        REQUIRE(endBytes[0] == 0xAD);
        REQUIRE(endBytes[1] == 0xDE);
    } else {
        REQUIRE(endBytes[0] == 0xDE);
        REQUIRE(endBytes[1] == 0xAD);
    }
}

TEST_CASE("Endianness convertion") {
    uint16_t u16data[2] = {0xDEAD, 0xADDE};
    int16_t s16data[2] = {0x1234, 0x3412};
    uint32_t u32data[2] = {0xDEADBEEF, 0xEFBEADDE};
    int32_t s32data[2] = {0x55ADBE44, 0x44BEAD55};
    uint64_t u64data[2] = {0xCAFED00DDEADBEEF, 0xEFBEADDE0DD0FECA};
    int64_t s64data[2] = {0x11FED00DDEADBE11, 0x11BEADDE0DD0FE11};

    CHECK(appfw::swapByteOrder(u16data[0]) == u16data[1]);
    CHECK(appfw::swapByteOrder(s16data[0]) == s16data[1]);
    CHECK(appfw::swapByteOrder(u32data[0]) == u32data[1]);
    CHECK(appfw::swapByteOrder(s32data[0]) == s32data[1]);
    CHECK(appfw::swapByteOrder(u64data[0]) == u64data[1]);
    CHECK(appfw::swapByteOrder(s64data[0]) == s64data[1]);

    bool idx = !appfw::isLittleEndian();
    CHECK(appfw::littleEndianSwap(u16data[0]) == u16data[idx]);
    CHECK(appfw::littleEndianSwap(s16data[0]) == s16data[idx]);
    CHECK(appfw::littleEndianSwap(u32data[0]) == u32data[idx]);
    CHECK(appfw::littleEndianSwap(s32data[0]) == s32data[idx]);
    CHECK(appfw::littleEndianSwap(u64data[0]) == u64data[idx]);
    CHECK(appfw::littleEndianSwap(s64data[0]) == s64data[idx]);

    idx = !idx;

    CHECK(appfw::bigEndianSwap(u16data[0]) == u16data[idx]);
    CHECK(appfw::bigEndianSwap(s16data[0]) == s16data[idx]);
    CHECK(appfw::bigEndianSwap(u32data[0]) == u32data[idx]);
    CHECK(appfw::bigEndianSwap(s32data[0]) == s32data[idx]);
    CHECK(appfw::bigEndianSwap(u64data[0]) == u64data[idx]);
    CHECK(appfw::bigEndianSwap(s64data[0]) == s64data[idx]);
}
