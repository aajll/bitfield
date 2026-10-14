/**
 * SPDX-License-Identifier: MIT
 *
 * @file: bitfield.c
 *
 * @brief
 *    Implementation of portable bit manipulation library.
 */

#include "bitfield.h"

/* ── 16-bit operations ──────────────────────────────────────────────────────
 *
 * Internal accumulator: uint32_t (one word wider than uint16_t).
 * This keeps all shift operations well-defined per MISRA C:2012 Rule 12.2:
 * the maximum shift count for 16-bit ops is 16, which is safely within the
 * 32-bit accumulator's width of 32.
 */

void
bitfield16_set(uint16_t *data, uint_least8_t bit_pos, bool value)
{
        if ((data == NULL) || (bit_pos >= 16U)) {
                return;
        }

        if (value) {
                *data |= (uint16_t)((uint16_t)1U << bit_pos);
        } else {
                *data &= (uint16_t)(~((uint16_t)1U << bit_pos));
        }
}

bool
bitfield16_get(const uint16_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 16U)) {
                return false;
        }

        return (bool)((*data & (uint16_t)((uint16_t)1U << bit_pos)) != 0U);
}

void
bitfield16_clear(uint16_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 16U)) {
                return;
        }

        *data &= (uint16_t)(~((uint16_t)1U << bit_pos));
}

void
bitfield16_set_range(uint16_t *data, uint_least8_t start, uint_least8_t end,
                     uint32_t value)
{
        uint32_t mask;
        uint_least8_t width;

        if ((data == NULL) || (start > end) || (end >= 16U)) {
                return;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint32_t)1U << width) - (uint32_t)1U) << start;

        *data = (uint16_t)((((uint32_t)*data) & (~mask))
                           | ((value << start) & mask));
}

uint32_t
bitfield16_get_range(const uint16_t *data, uint_least8_t start,
                     uint_least8_t end)
{
        uint32_t mask;
        uint_least8_t width;

        if ((data == NULL) || (start > end) || (end >= 16U)) {
                return 0U;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint32_t)1U << width) - (uint32_t)1U) << start;

        return (uint32_t)((((uint32_t)*data) & mask) >> start);
}

uint_least8_t
bitfield16_count_set(const uint16_t *data, uint_least8_t num_bits)
{
        uint_least8_t count = 0U;
        uint32_t value;

        if ((data == NULL) || (num_bits > 16U)) {
                return 0U;
        }

        value = (uint32_t)*data;

        for (uint_least8_t i = 0U; i < num_bits; i++) {
                if ((value & ((uint32_t)1U << i)) != 0U) {
                        count++;
                }
        }

        return count;
}

uint_least8_t
bitfield16_count_unset(const uint16_t *data, uint_least8_t num_bits)
{
        if ((data == NULL) || (num_bits > 16U)) {
                return 0U;
        }

        return (uint_least8_t)(num_bits - bitfield16_count_set(data, num_bits));
}

void
bitfield16_pack(const uint_least8_t *src, uint_least8_t src_len, uint16_t *dst,
                uint_least8_t dst_len)
{
        uint32_t accum = 0U;
        uint_least8_t bits = 0U;
        uint_least8_t di = 0U;

        if ((src == NULL) || (dst == NULL)) {
                return;
        }

        for (uint_least8_t i = 0U; i < src_len; i++) {
                accum |= ((uint32_t)src[i] & 0xFFU) << bits;
                bits = (uint_least8_t)(bits + 8U);

                while ((bits >= 16U) && (di < dst_len)) {
                        dst[di] = (uint16_t)(accum & 0xFFFFU);
                        di++;
                        accum >>= 16U;
                        bits = (uint_least8_t)(bits - 16U);
                }

                if (di >= dst_len) {
                        return;
                }
        }

        if ((bits != 0U) && (di < dst_len)) {
                dst[di] = (uint16_t)(accum & 0xFFFFU);
        }
}

void
bitfield16_unpack(const uint16_t *src, uint_least8_t src_len,
                  uint_least8_t *dst, uint_least8_t dst_len)
{
        uint32_t accum = 0U;
        uint_least8_t bits = 0U;
        uint_least8_t di = 0U;

        if ((src == NULL) || (dst == NULL)) {
                return;
        }

        for (uint_least8_t i = 0U; i < src_len; i++) {
                accum |= ((uint32_t)src[i]) << bits;
                bits = (uint_least8_t)(bits + 16U);

                while ((bits >= 8U) && (di < dst_len)) {
                        dst[di] = (uint_least8_t)(accum & 0xFFU);
                        di++;
                        accum >>= 8U;
                        bits = (uint_least8_t)(bits - 8U);
                }

                if (di >= dst_len) {
                        return;
                }
        }
}

/* ── 32-bit operations ──────────────────────────────────────────────────────
 *
 * Internal accumulator: uint64_t (one word wider than uint32_t).
 * The maximum shift count for 32-bit ops is 32, which is safely within the
 * 64-bit accumulator's width of 64.
 */

void
bitfield32_set(uint32_t *data, uint_least8_t bit_pos, bool value)
{
        if ((data == NULL) || (bit_pos >= 32U)) {
                return;
        }

        if (value) {
                *data |= (uint32_t)((uint32_t)1U << bit_pos);
        } else {
                *data &= (uint32_t)(~((uint32_t)1U << bit_pos));
        }
}

bool
bitfield32_get(const uint32_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 32U)) {
                return false;
        }

        return (bool)((*data & (uint32_t)((uint32_t)1U << bit_pos)) != 0U);
}

void
bitfield32_clear(uint32_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 32U)) {
                return;
        }

        *data &= (uint32_t)(~((uint32_t)1U << bit_pos));
}

void
bitfield32_set_range(uint32_t *data, uint_least8_t start, uint_least8_t end,
                     uint32_t value)
{
        uint64_t mask;
        uint_least8_t width;

        if ((data == NULL) || (start > end) || (end >= 32U)) {
                return;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint64_t)1ULL << width) - (uint64_t)1ULL) << start;

        *data = (uint32_t)((((uint64_t)*data) & (~mask))
                           | (((uint64_t)value << start) & mask));
}

uint32_t
bitfield32_get_range(const uint32_t *data, uint_least8_t start,
                     uint_least8_t end)
{
        uint64_t mask;
        uint_least8_t width;

        if ((data == NULL) || (start > end) || (end >= 32U)) {
                return 0U;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint64_t)1ULL << width) - (uint64_t)1ULL) << start;

        return (uint32_t)((((uint64_t)*data) & mask) >> start);
}

uint_least8_t
bitfield32_count_set(const uint32_t *data, uint_least8_t num_bits)
{
        uint_least8_t count = 0U;
        uint64_t value;

        if ((data == NULL) || (num_bits > 32U)) {
                return 0U;
        }

        value = (uint64_t)*data;

        for (uint_least8_t i = 0U; i < num_bits; i++) {
                if ((value & ((uint64_t)1ULL << i)) != 0U) {
                        count++;
                }
        }

        return count;
}

uint_least8_t
bitfield32_count_unset(const uint32_t *data, uint_least8_t num_bits)
{
        if ((data == NULL) || (num_bits > 32U)) {
                return 0U;
        }

        return (uint_least8_t)(num_bits - bitfield32_count_set(data, num_bits));
}

void
bitfield32_pack(const uint_least8_t *src, uint_least8_t src_len, uint32_t *dst,
                uint_least8_t dst_len)
{
        uint64_t accum = 0U;
        uint_least8_t bits = 0U;
        uint_least8_t di = 0U;

        if ((src == NULL) || (dst == NULL)) {
                return;
        }

        for (uint_least8_t i = 0U; i < src_len; i++) {
                accum |= ((uint64_t)src[i] & 0xFFU) << bits;
                bits = (uint_least8_t)(bits + 8U);

                while ((bits >= 32U) && (di < dst_len)) {
                        dst[di] = (uint32_t)(accum & 0xFFFFFFFFU);
                        di++;
                        accum >>= 32U;
                        bits = (uint_least8_t)(bits - 32U);
                }

                if (di >= dst_len) {
                        return;
                }
        }

        if ((bits != 0U) && (di < dst_len)) {
                dst[di] = (uint32_t)(accum & 0xFFFFFFFFU);
        }
}

void
bitfield32_unpack(const uint32_t *src, uint_least8_t src_len,
                  uint_least8_t *dst, uint_least8_t dst_len)
{
        uint64_t accum = 0U;
        uint_least8_t bits = 0U;
        uint_least8_t di = 0U;

        if ((src == NULL) || (dst == NULL)) {
                return;
        }

        for (uint_least8_t i = 0U; i < src_len; i++) {
                accum |= ((uint64_t)src[i]) << bits;
                bits = (uint_least8_t)(bits + 32U);

                while ((bits >= 8U) && (di < dst_len)) {
                        dst[di] = (uint_least8_t)(accum & 0xFFU);
                        di++;
                        accum >>= 8U;
                        bits = (uint_least8_t)(bits - 8U);
                }

                if (di >= dst_len) {
                        return;
                }
        }
}

/* ── CRC ────────────────────────────────────────────────────────────────────*/

uint16_t
bitfield_crc16(const uint_least8_t *data, uint16_t length, uint16_t polynomial)
{
        uint16_t crc = 0xFFFFU;

        if (data == NULL) {
                return 0U;
        }

        for (uint16_t i = 0U; i < length; i++) {
                crc ^= (uint16_t)((uint16_t)(data[i] & 0xFFU) << 8U);

                for (uint_least8_t j = 0U; j < 8U; j++) {
                        if ((crc & 0x8000U) != 0U) {
                                crc = (uint16_t)((crc << 1U) ^ polynomial);
                        } else {
                                crc = (uint16_t)(crc << 1U);
                        }
                }
        }

        return crc;
}
