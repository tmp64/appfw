/*
 * MIT License
 *
 * Copyright (c) 2021 Jeremy LAMBERT (SystemGlitch)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * Based on https://github.com/System-Glitch/SHA256
 */
#include <appfw/sha256.h>
#include <cstring>
#include <iomanip>
#include <sstream>

appfw::SHA256::SHA256() : m_blocklen(0), m_bitlen(0) {
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
}

void appfw::SHA256::update(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        m_data[m_blocklen++] = data[i];
        if (m_blocklen == 64) {
            transform();

            // End of the block
            m_bitlen += 512;
            m_blocklen = 0;
        }
    }
}

void appfw::SHA256::update(const std::string &data) {
    update(reinterpret_cast<const uint8_t *>(data.c_str()), data.size());
}

appfw::SHA256::Digest appfw::SHA256::digest() {
    Digest hash;

    pad();
    revert(hash.data());

    return hash;
}

uint32_t appfw::SHA256::rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

uint32_t appfw::SHA256::choose(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }

uint32_t appfw::SHA256::majority(uint32_t a, uint32_t b, uint32_t c) { return (a & (b | c)) | (b & c); }

uint32_t appfw::SHA256::sig0(uint32_t x) { return appfw::SHA256::rotr(x, 7) ^ appfw::SHA256::rotr(x, 18) ^ (x >> 3); }

uint32_t appfw::SHA256::sig1(uint32_t x) { return appfw::SHA256::rotr(x, 17) ^ appfw::SHA256::rotr(x, 19) ^ (x >> 10); }

void appfw::SHA256::transform() {
    uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
    uint32_t state[8];

    for (uint8_t i = 0, j = 0; i < 16; i++, j += 4) { // Split data in 32 bit blocks for the 16 first words
        m[i] = (m_data[j] << 24) | (m_data[j + 1] << 16) | (m_data[j + 2] << 8) | (m_data[j + 3]);
    }

    for (uint8_t k = 16; k < 64; k++) { // Remaining 48 blocks
        m[k] = appfw::SHA256::sig1(m[k - 2]) + m[k - 7] + appfw::SHA256::sig0(m[k - 15]) + m[k - 16];
    }

    for (uint8_t i = 0; i < 8; i++) {
        state[i] = m_state[i];
    }

    for (uint8_t i = 0; i < 64; i++) {
        maj = appfw::SHA256::majority(state[0], state[1], state[2]);
        xorA = appfw::SHA256::rotr(state[0], 2) ^ appfw::SHA256::rotr(state[0], 13) ^ appfw::SHA256::rotr(state[0], 22);

        ch = choose(state[4], state[5], state[6]);

        xorE = appfw::SHA256::rotr(state[4], 6) ^ appfw::SHA256::rotr(state[4], 11) ^ appfw::SHA256::rotr(state[4], 25);

        sum = m[i] + K[i] + state[7] + ch + xorE;
        newA = xorA + maj + sum;
        newE = state[3] + sum;

        state[7] = state[6];
        state[6] = state[5];
        state[5] = state[4];
        state[4] = newE;
        state[3] = state[2];
        state[2] = state[1];
        state[1] = state[0];
        state[0] = newA;
    }

    for (uint8_t i = 0; i < 8; i++) {
        m_state[i] += state[i];
    }
}

void appfw::SHA256::pad() {

    uint64_t i = m_blocklen;
    uint8_t end = m_blocklen < 56 ? 56 : 64;

    m_data[i++] = 0x80; // Append a bit 1
    while (i < end) {
        m_data[i++] = 0x00; // Pad with zeros
    }

    if (m_blocklen >= 56) {
        transform();
        memset(m_data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    m_bitlen += m_blocklen * 8;
    m_data[63] = (uint8_t)(m_bitlen);
    m_data[62] = (uint8_t)(m_bitlen >> 8);
    m_data[61] = (uint8_t)(m_bitlen >> 16);
    m_data[60] = (uint8_t)(m_bitlen >> 24);
    m_data[59] = (uint8_t)(m_bitlen >> 32);
    m_data[58] = (uint8_t)(m_bitlen >> 40);
    m_data[57] = (uint8_t)(m_bitlen >> 48);
    m_data[56] = (uint8_t)(m_bitlen >> 56);
    transform();
}

void appfw::SHA256::revert(uint8_t *hash) {
    // SHA uses big endian byte ordering
    // Revert all bytes
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            hash[i + (j * 4)] = (m_state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}

std::string appfw::SHA256::toString(const uint8_t *digest) {
    std::stringstream s;
    s << std::setfill('0') << std::hex;

    for (uint8_t i = 0; i < 32; i++) {
        s << std::setw(2) << (unsigned int)digest[i];
    }

    return s.str();
}
