/**
 * SPDX-License-Identifier: MIT
 *
 * @file: bitfield.c
 *
 * @brief
 *    Implementation of portable bit manipulation library.
 */

#include "bitfield.h"

/* Octet and word counts in pack and unpack. A length is a uint_least8_t,
 * which is 16 bits wide where CHAR_BIT is 16, so an unpack count reaches
 * four times 65535 and needs a type wider than 16 bits.
 */
typedef uint_least32_t bitfield_count_t;

_Static_assert(
    ((bitfield_count_t)UINT_LEAST32_MAX / 4U) >= UINT_LEAST8_MAX,
    "bitfield_count_t must hold four times any uint_least8_t length");

/* ── 16-bit operations ──────────────────────────────────────────────────────
 *
 * Range masks use a uint32_t accumulator, one word wider than uint16_t, so
 * the widest shift (16) stays below the accumulator width (MISRA C:2023
 * Rule 12.2).
 */

bitfield_status_t
bitfield16_set(uint16_t *data, uint_least8_t bit_pos, bool value)
{
        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if (bit_pos >= 16U) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        if (value) {
                *data |= (uint16_t)((uint16_t)1U << bit_pos);
        } else {
                *data &= (uint16_t)(~((uint16_t)1U << bit_pos));
        }
        return BITFIELD_STATUS_OK;
}

bool
bitfield16_get(const uint16_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 16U)) {
                return false;
        }

        return (bool)((*data & (uint16_t)((uint16_t)1U << bit_pos)) != 0U);
}

bitfield_status_t
bitfield16_clear(uint16_t *data, uint_least8_t bit_pos)
{
        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if (bit_pos >= 16U) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        *data &= (uint16_t)(~((uint16_t)1U << bit_pos));
        return BITFIELD_STATUS_OK;
}

bitfield_status_t
bitfield16_set_range(uint16_t *data, uint_least8_t start, uint_least8_t end,
                     uint32_t value)
{
        uint32_t mask;
        uint_least8_t width;

        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if ((start > end) || (end >= 16U)) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint32_t)1U << width) - (uint32_t)1U) << start;

        *data = (uint16_t)((((uint32_t)*data) & (~mask))
                           | ((value << start) & mask));
        return BITFIELD_STATUS_OK;
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

bitfield_status_t
bitfield16_pack(const uint_least8_t *src, uint_least8_t src_len, uint16_t *dst,
                uint_least8_t dst_len)
{
        if ((src == NULL) || (dst == NULL)) {
                return BITFIELD_STATUS_NULL_POINTER;
        }

        const bitfield_count_t words =
            ((bitfield_count_t)src_len + (2U - 1U)) / 2U;
        const bitfield_count_t n = (words < dst_len) ? words : dst_len;

        for (bitfield_count_t w = 0U; w < n; w++) {
                const bitfield_count_t i = 2U * w;
                uint32_t word = (uint32_t)src[i] & 0xFFU;

                if ((i + 1U) < src_len) {
                        word |= ((uint32_t)src[i + 1U] & 0xFFU) << 8U;
                }
                dst[w] = (uint16_t)word;
        }

        return (words > dst_len) ? BITFIELD_STATUS_TRUNCATED
                                 : BITFIELD_STATUS_OK;
}

bitfield_status_t
bitfield16_unpack(const uint16_t *src, uint_least8_t src_len,
                  uint_least8_t *dst, uint_least8_t dst_len)
{
        if ((src == NULL) || (dst == NULL)) {
                return BITFIELD_STATUS_NULL_POINTER;
        }

        const bitfield_count_t octets = 2U * (bitfield_count_t)src_len;
        const bitfield_count_t n = (octets < dst_len) ? octets : dst_len;

        for (bitfield_count_t b = 0U; b < n; b++) {
                dst[b] =
                    (uint_least8_t)((src[b / 2U] >> (8U * (b % 2U))) & 0xFFU);
        }

        return (octets > dst_len) ? BITFIELD_STATUS_TRUNCATED
                                  : BITFIELD_STATUS_OK;
}

/* ── 32-bit operations ──────────────────────────────────────────────────────
 *
 * Range masks use a uint64_t accumulator, one word wider than uint32_t, so
 * the widest shift (32) stays below the accumulator width.
 */

bitfield_status_t
bitfield32_set(uint32_t *data, uint_least8_t bit_pos, bool value)
{
        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if (bit_pos >= 32U) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        if (value) {
                *data |= (uint32_t)((uint32_t)1U << bit_pos);
        } else {
                *data &= (uint32_t)(~((uint32_t)1U << bit_pos));
        }
        return BITFIELD_STATUS_OK;
}

bool
bitfield32_get(const uint32_t *data, uint_least8_t bit_pos)
{
        if ((data == NULL) || (bit_pos >= 32U)) {
                return false;
        }

        return (bool)((*data & (uint32_t)((uint32_t)1U << bit_pos)) != 0U);
}

bitfield_status_t
bitfield32_clear(uint32_t *data, uint_least8_t bit_pos)
{
        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if (bit_pos >= 32U) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        *data &= (uint32_t)(~((uint32_t)1U << bit_pos));
        return BITFIELD_STATUS_OK;
}

bitfield_status_t
bitfield32_set_range(uint32_t *data, uint_least8_t start, uint_least8_t end,
                     uint32_t value)
{
        uint64_t mask;
        uint_least8_t width;

        if (data == NULL) {
                return BITFIELD_STATUS_NULL_POINTER;
        }
        if ((start > end) || (end >= 32U)) {
                return BITFIELD_STATUS_OUT_OF_RANGE;
        }

        width = (uint_least8_t)(end - start + 1U);
        mask = (((uint64_t)1ULL << width) - (uint64_t)1ULL) << start;

        *data = (uint32_t)((((uint64_t)*data) & (~mask))
                           | (((uint64_t)value << start) & mask));
        return BITFIELD_STATUS_OK;
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

bitfield_status_t
bitfield32_pack(const uint_least8_t *src, uint_least8_t src_len, uint32_t *dst,
                uint_least8_t dst_len)
{
        if ((src == NULL) || (dst == NULL)) {
                return BITFIELD_STATUS_NULL_POINTER;
        }

        const bitfield_count_t words =
            ((bitfield_count_t)src_len + (4U - 1U)) / 4U;
        const bitfield_count_t n = (words < dst_len) ? words : dst_len;

        for (bitfield_count_t w = 0U; w < n; w++) {
                const bitfield_count_t i = 4U * w;
                uint32_t word = 0U;

                for (bitfield_count_t k = 0U; ((i + k) < src_len) && (k < 4U);
                     k++) {
                        word |= ((uint32_t)src[i + k] & 0xFFU) << (8U * k);
                }
                dst[w] = word;
        }

        return (words > dst_len) ? BITFIELD_STATUS_TRUNCATED
                                 : BITFIELD_STATUS_OK;
}

bitfield_status_t
bitfield32_unpack(const uint32_t *src, uint_least8_t src_len,
                  uint_least8_t *dst, uint_least8_t dst_len)
{
        if ((src == NULL) || (dst == NULL)) {
                return BITFIELD_STATUS_NULL_POINTER;
        }

        const bitfield_count_t octets = 4U * (bitfield_count_t)src_len;
        const bitfield_count_t n = (octets < dst_len) ? octets : dst_len;

        for (bitfield_count_t b = 0U; b < n; b++) {
                dst[b] =
                    (uint_least8_t)((src[b / 4U] >> (8U * (b % 4U))) & 0xFFU);
        }

        return (octets > dst_len) ? BITFIELD_STATUS_TRUNCATED
                                  : BITFIELD_STATUS_OK;
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
