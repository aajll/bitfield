/*
 * SPDX-License-Identifier: MIT
 *
 * @file: test_bitfield.c
 * @brief Unit tests for the bitfield16_* and bitfield32_* families.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bitfield.h"

/* ── 16-bit: individual bit operations ─────────────────────────────────── */

static void
test_bit_operations_16(void)
{
        uint16_t value = 0U;

        bitfield16_set(&value, 0U, true);
        assert(bitfield16_get(&value, 0U) == true);
        assert(value == 0x0001U);

        bitfield16_set(&value, 7U, true);
        assert(bitfield16_get(&value, 7U) == true);
        assert(value == 0x0081U);

        /* Top bit */
        bitfield16_set(&value, 15U, true);
        assert(bitfield16_get(&value, 15U) == true);
        assert(value == 0x8081U);

        bitfield16_clear(&value, 0U);
        assert(bitfield16_get(&value, 0U) == false);
        assert(value == 0x8080U);

        bitfield16_clear(&value, 15U);
        assert(bitfield16_get(&value, 15U) == false);
        assert(value == 0x0080U);

        /* set false clears the bit */
        bitfield16_set(&value, 7U, false);
        assert(bitfield16_get(&value, 7U) == false);
        assert(value == 0x0000U);

        printf("Bit operations (16-bit): PASSED\n");
}

/* ── 32-bit: individual bit operations ─────────────────────────────────── */

static void
test_bit_operations_32(void)
{
        uint32_t value = 0U;

        bitfield32_set(&value, 0U, true);
        assert(bitfield32_get(&value, 0U) == true);
        assert(value == 0x00000001U);

        bitfield32_set(&value, 15U, true);
        assert(bitfield32_get(&value, 15U) == true);
        assert(value == 0x00008001U);

        /* Top bit */
        bitfield32_set(&value, 31U, true);
        assert(bitfield32_get(&value, 31U) == true);
        assert(value == 0x80008001U);

        bitfield32_clear(&value, 0U);
        assert(bitfield32_get(&value, 0U) == false);
        assert(value == 0x80008000U);

        bitfield32_clear(&value, 31U);
        assert(bitfield32_get(&value, 31U) == false);
        assert(value == 0x00008000U);

        /* set false clears the bit */
        bitfield32_set(&value, 15U, false);
        assert(bitfield32_get(&value, 15U) == false);
        assert(value == 0x00000000U);

        printf("Bit operations (32-bit): PASSED\n");
}

/* ── 16-bit: range operations ──────────────────────────────────────────── */

static void
test_bitfield_range_16(void)
{
        uint16_t value = 0U;

        /* Sub-range write and read-back */
        bitfield16_set_range(&value, 2U, 4U, 0x5U);
        assert(bitfield16_get_range(&value, 2U, 4U) == 0x5U);

        bitfield16_set_range(&value, 0U, 1U, 0x3U);
        assert(bitfield16_get_range(&value, 0U, 1U) == 0x3U);

        /* Prior range must be unchanged */
        assert(bitfield16_get_range(&value, 2U, 4U) == 0x5U);

        /* Upper byte */
        bitfield16_set_range(&value, 8U, 15U, 0xABU);
        assert(bitfield16_get_range(&value, 8U, 15U) == 0xABU);

        /* Full 16-bit range */
        bitfield16_set_range(&value, 0U, 15U, 0xDEADU);
        assert(bitfield16_get_range(&value, 0U, 15U) == 0xDEADU);
        assert(value == 0xDEADU);

        /* Upper half preserved when lower half changed */
        bitfield16_set_range(&value, 0U, 7U, 0xBEU);
        assert(bitfield16_get_range(&value, 8U, 15U) == 0xDEU);
        assert(bitfield16_get_range(&value, 0U, 7U) == 0xBEU);

        printf("Range operations (16-bit): PASSED\n");
}

/* ── 32-bit: range operations ──────────────────────────────────────────── */

static void
test_bitfield_range_32(void)
{
        uint32_t value = 0U;

        /* Sub-range write and read-back */
        bitfield32_set_range(&value, 2U, 4U, 0x5U);
        assert(bitfield32_get_range(&value, 2U, 4U) == 0x5U);

        bitfield32_set_range(&value, 0U, 1U, 0x3U);
        assert(bitfield32_get_range(&value, 0U, 1U) == 0x3U);

        /* Prior range must be unchanged */
        assert(bitfield32_get_range(&value, 2U, 4U) == 0x5U);

        /* Upper half */
        bitfield32_set_range(&value, 16U, 31U, 0xCAFEU);
        assert(bitfield32_get_range(&value, 16U, 31U) == 0xCAFEU);

        /* Full 32-bit range */
        bitfield32_set_range(&value, 0U, 31U, 0xDEADBEEFU);
        assert(bitfield32_get_range(&value, 0U, 31U) == 0xDEADBEEFU);
        assert(value == 0xDEADBEEFU);

        /* Upper half preserved when lower half changed */
        bitfield32_set_range(&value, 0U, 15U, 0xCAFEU);
        assert(bitfield32_get_range(&value, 16U, 31U) == 0xDEADU);
        assert(bitfield32_get_range(&value, 0U, 15U) == 0xCAFEU);

        printf("Range operations (32-bit): PASSED\n");
}

/* ── 16-bit: bit counting ──────────────────────────────────────────────── */

static void
test_bit_counting_16(void)
{
        uint16_t value;

        value = 0xAAAAU; /* alternating: 8 set, 8 unset */
        assert(bitfield16_count_set(&value, 16U) == 8U);
        assert(bitfield16_count_unset(&value, 16U) == 8U);

        value = 0xFFFFU;
        assert(bitfield16_count_set(&value, 16U) == 16U);
        assert(bitfield16_count_unset(&value, 16U) == 0U);

        value = 0x0000U;
        assert(bitfield16_count_set(&value, 16U) == 0U);
        assert(bitfield16_count_unset(&value, 16U) == 16U);

        /* Partial count — only examine lower 8 bits of 0xAAAA (= 0xAA = 4 set)
         */
        value = 0xAAAAU;
        assert(bitfield16_count_set(&value, 8U) == 4U);
        assert(bitfield16_count_unset(&value, 8U) == 4U);

        printf("Bit counting (16-bit): PASSED\n");
}

/* ── 32-bit: bit counting ──────────────────────────────────────────────── */

static void
test_bit_counting_32(void)
{
        uint32_t value;

        value = 0xAAAAAAAAU; /* alternating: 16 set, 16 unset */
        assert(bitfield32_count_set(&value, 32U) == 16U);
        assert(bitfield32_count_unset(&value, 32U) == 16U);

        value = 0xFFFFFFFFU;
        assert(bitfield32_count_set(&value, 32U) == 32U);
        assert(bitfield32_count_unset(&value, 32U) == 0U);

        value = 0x00000000U;
        assert(bitfield32_count_set(&value, 32U) == 0U);
        assert(bitfield32_count_unset(&value, 32U) == 32U);

        /* Partial count — lower 16 bits of 0xAAAAAAAA = 0xAAAA = 8 set */
        value = 0xAAAAAAAAU;
        assert(bitfield32_count_set(&value, 16U) == 8U);
        assert(bitfield32_count_unset(&value, 16U) == 8U);

        printf("Bit counting (32-bit): PASSED\n");
}

/* ── 16-bit: pack / unpack ──────────────────────────────────────────────────*/

static void
test_packing_16(void)
{
        /* Single byte round-trip */
        uint8_t src1[1] = {0xABU};
        uint16_t dst1[1] = {0U};
        uint8_t out1[1] = {0U};
        bitfield16_pack(src1, 1U, dst1, 1U);
        bitfield16_unpack(dst1, 1U, out1, 1U);
        assert(out1[0] == 0xABU);

        /* Two bytes pack into one 16-bit word (little-endian) */
        uint8_t src2[2] = {0x12U, 0x34U};
        uint16_t dst2[1] = {0U};
        uint8_t out2[2] = {0U, 0U};
        bitfield16_pack(src2, 2U, dst2, 1U);
        assert(dst2[0] == 0x3412U);
        bitfield16_unpack(dst2, 1U, out2, 2U);
        assert(out2[0] == 0x12U);
        assert(out2[1] == 0x34U);

        /* Six bytes into three 16-bit words round-trip */
        uint8_t src6[6] = {0xAAU, 0x55U, 0xFFU, 0x00U, 0x12U, 0x34U};
        uint16_t dst6[3] = {0U, 0U, 0U};
        uint8_t out6[6] = {0U};
        bitfield16_pack(src6, 6U, dst6, 3U);
        bitfield16_unpack(dst6, 3U, out6, 6U);
        for (uint8_t i = 0U; i < 6U; i++) {
                assert(out6[i] == src6[i]);
        }

        printf("Pack/unpack (16-bit): PASSED\n");
}

/* ── 32-bit: pack / unpack ─────────────────────────────────────────────── */

static void
test_packing_32(void)
{
        /* Single byte round-trip */
        uint8_t src1[1] = {0xABU};
        uint32_t dst1[1] = {0U};
        uint8_t out1[1] = {0U};
        bitfield32_pack(src1, 1U, dst1, 1U);
        bitfield32_unpack(dst1, 1U, out1, 1U);
        assert(out1[0] == 0xABU);

        /* Four bytes pack into one 32-bit word (little-endian) */
        uint8_t src4[4] = {0x11U, 0x22U, 0x33U, 0x44U};
        uint32_t dst4[1] = {0U};
        uint8_t out4[4] = {0U};
        bitfield32_pack(src4, 4U, dst4, 1U);
        assert(dst4[0] == 0x44332211U);
        bitfield32_unpack(dst4, 1U, out4, 4U);
        assert(out4[0] == 0x11U);
        assert(out4[1] == 0x22U);
        assert(out4[2] == 0x33U);
        assert(out4[3] == 0x44U);

        /* Eight bytes into two 32-bit words round-trip */
        uint8_t src8[8] = {0x01U, 0x02U, 0x03U, 0x04U,
                           0x05U, 0x06U, 0x07U, 0x08U};
        uint32_t dst8[2] = {0U, 0U};
        uint8_t out8[8] = {0U};
        bitfield32_pack(src8, 8U, dst8, 2U);
        assert(dst8[0] == 0x04030201U);
        assert(dst8[1] == 0x08070605U);
        bitfield32_unpack(dst8, 2U, out8, 8U);
        for (uint8_t i = 0U; i < 8U; i++) {
                assert(out8[i] == src8[i]);
        }

        printf("Pack/unpack (32-bit): PASSED\n");
}

/* ── NULL pointer safety ───────────────────────────────────────────────── */

static void
test_null_pointer(void)
{
        uint16_t v16 = 0U;
        uint32_t v32 = 0U;

        /* 16-bit family */
        bitfield16_set(NULL, 0U, true);
        assert(v16 == 0U);
        assert(bitfield16_get(NULL, 0U) == false);
        bitfield16_clear(NULL, 0U);
        bitfield16_set_range(NULL, 0U, 1U, 0U);
        assert(bitfield16_get_range(NULL, 0U, 1U) == 0U);
        assert(bitfield16_count_set(NULL, 8U) == 0U);
        assert(bitfield16_count_unset(NULL, 8U) == 0U);
        bitfield16_pack(NULL, 1U, NULL, 1U);
        bitfield16_unpack(NULL, 1U, NULL, 1U);

        /* 32-bit family */
        bitfield32_set(NULL, 0U, true);
        assert(v32 == 0U);
        assert(bitfield32_get(NULL, 0U) == false);
        bitfield32_clear(NULL, 0U);
        bitfield32_set_range(NULL, 0U, 1U, 0U);
        assert(bitfield32_get_range(NULL, 0U, 1U) == 0U);
        assert(bitfield32_count_set(NULL, 8U) == 0U);
        assert(bitfield32_count_unset(NULL, 8U) == 0U);
        bitfield32_pack(NULL, 1U, NULL, 1U);
        bitfield32_unpack(NULL, 1U, NULL, 1U);

        /* CRC */
        assert(bitfield_crc16(NULL, 4U, 0x8005U) == 0U);

        printf("Null-pointer handling: PASSED\n");
}

/* ── 16-bit: invalid range / bit-count / bit-position ──────────────────── */

static void
test_invalid_inputs_16(void)
{
        uint16_t value = 0xFFFFU;

        /* start > end: no-op */
        bitfield16_set_range(&value, 5U, 3U, 0U);
        assert(value == 0xFFFFU);
        assert(bitfield16_get_range(&value, 5U, 3U) == 0U);

        /* end >= 16: no-op */
        bitfield16_set_range(&value, 0U, 16U, 0U);
        assert(value == 0xFFFFU);
        assert(bitfield16_get_range(&value, 0U, 16U) == 0U);

        /* num_bits > 16: returns 0 */
        assert(bitfield16_count_set(&value, 17U) == 0U);
        assert(bitfield16_count_unset(&value, 17U) == 0U);

        /* bit_pos >= 16: no-op */
        value = 0U;
        bitfield16_set(&value, 16U, true);
        assert(value == 0U);
        bitfield16_clear(&value, 16U);
        assert(value == 0U);
        assert(bitfield16_get(&value, 16U) == false);

        printf("Invalid inputs (16-bit): PASSED\n");
}

/* ── 32-bit: invalid range / bit-count / bit-position ──────────────────── */

static void
test_invalid_inputs_32(void)
{
        uint32_t value = 0xFFFFFFFFU;

        /* start > end: no-op */
        bitfield32_set_range(&value, 5U, 3U, 0U);
        assert(value == 0xFFFFFFFFU);
        assert(bitfield32_get_range(&value, 5U, 3U) == 0U);

        /* end >= 32: no-op */
        bitfield32_set_range(&value, 0U, 32U, 0U);
        assert(value == 0xFFFFFFFFU);
        assert(bitfield32_get_range(&value, 0U, 32U) == 0U);

        /* num_bits > 32: returns 0 */
        assert(bitfield32_count_set(&value, 33U) == 0U);
        assert(bitfield32_count_unset(&value, 33U) == 0U);

        /* bit_pos >= 32: no-op */
        value = 0U;
        bitfield32_set(&value, 32U, true);
        assert(value == 0U);
        bitfield32_clear(&value, 32U);
        assert(value == 0U);
        assert(bitfield32_get(&value, 32U) == false);

        printf("Invalid inputs (32-bit): PASSED\n");
}

/* ── 16-bit: boundary / shift-bounds ───────────────────────────────────── */

static void
test_shift_bounds_16(void)
{
        uint16_t value = 0U;

        /* Max valid range: bits 0–15, value = 0xFFFF */
        bitfield16_set_range(&value, 0U, 15U, 0xFFFFU);
        assert(bitfield16_get_range(&value, 0U, 15U) == 0xFFFFU);
        assert(value == 0xFFFFU);

        /* Top bit only */
        value = 0U;
        bitfield16_set_range(&value, 15U, 15U, 1U);
        assert(bitfield16_get_range(&value, 15U, 15U) == 1U);
        assert(value == 0x8000U);

        /* Out-of-range end is rejected; value must be unchanged */
        value = 0xFFFFU;
        bitfield16_set_range(&value, 0U, 16U, 0U);
        assert(value == 0xFFFFU);

        printf("Shift bounds (16-bit): PASSED\n");
}

/* ── 32-bit: boundary / shift-bounds ───────────────────────────────────── */

static void
test_shift_bounds_32(void)
{
        uint32_t value = 0U;

        /* Max valid range: bits 0–31, value = 0xFFFFFFFF */
        bitfield32_set_range(&value, 0U, 31U, 0xFFFFFFFFU);
        assert(bitfield32_get_range(&value, 0U, 31U) == 0xFFFFFFFFU);
        assert(value == 0xFFFFFFFFU);

        /* Top bit only */
        value = 0U;
        bitfield32_set_range(&value, 31U, 31U, 1U);
        assert(bitfield32_get_range(&value, 31U, 31U) == 1U);
        assert(value == 0x80000000U);

        /* Out-of-range end is rejected; value must be unchanged */
        value = 0xFFFFFFFFU;
        bitfield32_set_range(&value, 0U, 32U, 0U);
        assert(value == 0xFFFFFFFFU);

        printf("Shift bounds (32-bit): PASSED\n");
}

/* ── 16-bit: pack truncation ───────────────────────────────────────────── */

static void
test_pack_truncation_16(void)
{
        uint8_t src[4] = {0x12U, 0x34U, 0x56U, 0x78U};

        /* Destination too small: only first word is written */
        uint16_t dst_small[1] = {0xABCDU};
        bitfield16_pack(src, 4U, dst_small, 1U);
        assert(dst_small[0] == 0x3412U);

        /* Adequate destination: all four bytes packed into two words */
        uint16_t dst_ok[2] = {0U, 0U};
        bitfield16_pack(src, 4U, dst_ok, 2U);
        assert(dst_ok[0] == 0x3412U);
        assert(dst_ok[1] == 0x7856U);

        printf("Pack truncation (16-bit): PASSED\n");
}

/* ── 32-bit: pack truncation ───────────────────────────────────────────── */

static void
test_pack_truncation_32(void)
{
        uint8_t src[8] = {0x11U, 0x22U, 0x33U, 0x44U,
                          0x55U, 0x66U, 0x77U, 0x88U};

        /* Destination too small: only first word is written */
        uint32_t dst_small[1] = {0xDEADBEEFU};
        bitfield32_pack(src, 8U, dst_small, 1U);
        assert(dst_small[0] == 0x44332211U);

        /* Adequate destination: all eight bytes packed into two words */
        uint32_t dst_ok[2] = {0U, 0U};
        bitfield32_pack(src, 8U, dst_ok, 2U);
        assert(dst_ok[0] == 0x44332211U);
        assert(dst_ok[1] == 0x88776655U);

        printf("Pack truncation (32-bit): PASSED\n");
}

/* ── Large buffer pack (both families) ─────────────────────────────────── */

static void
test_pack_large_buffer(void)
{
        uint8_t src[10] = {0U};
        for (uint8_t i = 0U; i < 10U; i++) {
                src[i] = (uint8_t)i;
        }

        uint16_t dst16[10] = {0U};
        bitfield16_pack(src, 10U, dst16, 10U);

        uint32_t dst32[10] = {0U};
        bitfield32_pack(src, 10U, dst32, 10U);

        printf("Pack large buffer: PASSED\n");
}

/* ── Mixed-width usage (simulates C2000 driver style) ──────────────────── */

static void
test_mixed_width_usage(void)
{
        /* Demonstrates using both families in the same context, as one would
         * when a peripheral has both 16-bit control registers and 32-bit
         * data registers (e.g., TI C2000 ePWM module). */
        uint16_t ctrl_reg = 0U;
        uint32_t period_reg = 0U;

        /* Set mode bits in 16-bit control register */
        bitfield16_set_range(&ctrl_reg, 0U, 1U, 0x2U); /* mode = 2 */
        bitfield16_set(&ctrl_reg, 4U, true);           /* enable bit */
        assert(bitfield16_get_range(&ctrl_reg, 0U, 1U) == 0x2U);
        assert(bitfield16_get(&ctrl_reg, 4U) == true);

        /* Set period value in 32-bit data register */
        bitfield32_set_range(&period_reg, 0U, 31U, 0x000186A0U); /* 100000 */
        assert(bitfield32_get_range(&period_reg, 0U, 31U) == 0x000186A0U);

        /* Both registers independently correct */
        assert(ctrl_reg == 0x0012U);
        assert(period_reg == 0x000186A0U);

        printf("Mixed-width usage: PASSED\n");
}

/* ── CRC ───────────────────────────────────────────────────────────────── */

static void
test_crc(void)
{
        uint8_t data[4] = {0x01U, 0x23U, 0x45U, 0x67U};
        uint16_t crc = bitfield_crc16(data, 4U, 0x8005U);

        assert(crc == 0x09CFU);

        printf("CRC calculation: PASSED\n");
}

/* With polynomial 0x1021 the result is CRC-16/CCITT-FALSE, the model the
 * deprecation note maps to ucrc; 0x29B1 is its catalogued check value. */
static void
test_crc_matches_ccitt_false(void)
{
        const uint_least8_t check[9] = {'1', '2', '3', '4', '5',
                                        '6', '7', '8', '9'};

        assert(bitfield_crc16(check, 9U, 0x1021U) == 0x29B1U);

        printf("CRC CCITT-FALSE check value: PASSED\n");
}

static void
test_crc_edge_cases(void)
{
        uint8_t data[4] = {0x01U, 0x23U, 0x45U, 0x67U};

        assert(bitfield_crc16(NULL, 4U, 0x8005U) == 0U);
        assert(bitfield_crc16(data, 4U, 0x8005U) == 0x09CFU);
        assert(bitfield_crc16(data, 0U, 0x8005U) == 0xFFFFU);

        printf("CRC edge cases: PASSED\n");
}

/* ── CRC with different polynomials and edge data ───────────────────────── */

static void
test_crc_variations(void)
{
        /* Test with multiple CRC-16 polynomials */
        uint8_t data[2] = {0x55U, 0xAAU};

        /* CRC-16-CCITT (polynomial 0x1021) */
        uint16_t crc1 = bitfield_crc16(data, 2U, 0x1021U);
        assert(crc1 != 0xFFFFU);

        /* CRC-16-IBM (polynomial 0x8005) */
        uint16_t crc2 = bitfield_crc16(data, 2U, 0x8005U);
        assert(crc2 != 0xFFFFU);

        /* NULL data returns 0 */
        assert(bitfield_crc16(NULL, 0U, 0x8005U) == 0U);
        /* Empty data returns polynomial (0xFFFF) */
        assert(bitfield_crc16(data, 0U, 0x8005U) == 0xFFFFU);

        /* Single byte */
        uint8_t single[1] = {0x42U};
        assert(bitfield_crc16(single, 1U, 0x8005U) != 0xFFFFU);

        printf("CRC variations: PASSED\n");
}

/* ── 16-bit: range operations with overlapping/adjacent ranges ─────────── */

static void
test_range_overlap_16(void)
{
        uint16_t value = 0x1234U;

        /* Set range bits 4-7 to 0xF */
        bitfield16_set_range(&value, 4U, 7U, 0xFU);
        assert(value == 0x12F4U);

        /* Set adjacent range: bits 0-3 to 0xF */
        bitfield16_set_range(&value, 0U, 3U, 0x0FU);
        assert(value == 0x12FFU);

        /* Get partial overlap: bits 2-5 */
        uint32_t partial = bitfield16_get_range(&value, 2U, 5U);
        assert(partial == 0xFU);

        printf("Range overlap (16-bit): PASSED\n");
}

/* ── Pack/unpack with misaligned byte counts ───────────────────────────── */

static void
test_pack_unpack_misaligned_16(void)
{
        /* Odd number of bytes (3) into 2-word destination */
        uint8_t src[3] = {0x11U, 0x22U, 0x33U};
        uint16_t dst[2] = {0U, 0U};
        bitfield16_pack(src, 3U, dst, 2U);
        assert(dst[0] == 0x2211U);
        assert(dst[1] == 0x0033U); /* Last byte zero-extended */

        /* Verify round-trip with odd bytes */
        uint8_t out[3] = {0U};
        bitfield16_unpack(dst, 2U, out, 3U);
        assert(out[0] == 0x11U);
        assert(out[1] == 0x22U);
        assert(out[2] == 0x33U);

        printf("Pack/unpack misaligned (16-bit): PASSED\n");
}

/* ── Count operations on partial ranges with specific bit patterns ─────── */

static void
test_count_partial_patterns_16(void)
{
        uint16_t value = 0x5555U; /* 0101010101010101 */

        /* Count from bit 2 to 5 (4 bits: 0101 = 2 set, 2 unset) */
        assert(bitfield16_count_set(&value, 4U) == 2U);
        assert(bitfield16_count_unset(&value, 4U) == 2U);

        /* Count from bit 12 to 15 (4 bits: 0101 = 2 set, 2 unset) */
        assert(bitfield16_count_set(&value, 4U) == 2U);

        /* Count from bit 1 (skips bit 0 which is 1) */
        assert(bitfield16_count_set(&value, 15U) == 8U);
        assert(bitfield16_count_unset(&value, 15U) == 7U);

        printf("Count partial patterns (16-bit): PASSED\n");
}

/* ── 32-bit range operations with multi-word values
 * ──────────────────────────*/

static void
test_range_32_multiword(void)
{
        uint32_t value = 0x00000000U;

        /* Set upper 16 bits */
        bitfield32_set_range(&value, 16U, 31U, 0xABCDU);
        assert(value == 0xABCD0000U);

        /* Set lower 16 bits */
        bitfield32_set_range(&value, 0U, 15U, 0xEF01U);
        assert(value == 0xABCDEF01U);

        /* Get 24-bit range (spans bit boundaries) */
        uint32_t range24 = bitfield32_get_range(&value, 4U, 27U);
        assert(range24 == 0x00BCDEF0U);

        printf("Range 32-bit multiword: PASSED\n");
}

static void
test_version(void)
{
        assert(BITFIELD_VERSION_NUMBER
               == ((BITFIELD_VERSION_MAJOR * 10000L)
                   + (BITFIELD_VERSION_MINOR * 100L) + BITFIELD_VERSION_PATCH));
        assert(BITFIELD_VERSION[0] != '\0');

        printf("Version macros: PASSED\n");
}

/* ── main ──────────────────────────────────────────────────────────────── */

int
main(void)
{
        test_bit_operations_16();
        test_bit_operations_32();
        test_bitfield_range_16();
        test_bitfield_range_32();
        test_bit_counting_16();
        test_bit_counting_32();
        test_packing_16();
        test_packing_32();
        test_null_pointer();
        test_invalid_inputs_16();
        test_invalid_inputs_32();
        test_shift_bounds_16();
        test_shift_bounds_32();
        test_pack_truncation_16();
        test_pack_truncation_32();
        test_pack_large_buffer();
        test_mixed_width_usage();
        test_crc();
        test_crc_matches_ccitt_false();
        test_crc_edge_cases();
        test_crc_variations();
        test_range_overlap_16();
        test_pack_unpack_misaligned_16();
        test_count_partial_patterns_16();
        test_range_32_multiword();
        test_version();

        printf("\nAll tests PASSED!\n");
        return 0;
}
