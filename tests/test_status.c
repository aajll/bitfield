/*
 * SPDX-License-Identifier: MIT
 *
 * @file: test_status.c
 * @brief Status returned by the functions that modify a field or buffer:
 *        every rejection leaves the destination unchanged, and truncation
 *        fills the destination as far as it goes.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bitfield.h"

static void
test_bit_status_16(void)
{
        uint16_t v = 0xA5A5U;

        assert(bitfield16_set(NULL, 0U, true) == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield16_clear(NULL, 0U) == BITFIELD_STATUS_NULL_POINTER);

        assert(bitfield16_set(&v, 16U, true) == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(bitfield16_clear(&v, 16U) == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(v == 0xA5A5U);

        assert(bitfield16_set(&v, 15U, false) == BITFIELD_STATUS_OK);
        assert(bitfield16_clear(&v, 0U) == BITFIELD_STATUS_OK);
        assert(v == 0x25A4U);

        printf("Bit status (16-bit): PASSED\n");
}

static void
test_bit_status_32(void)
{
        uint32_t v = 0xA5A5A5A5UL;

        assert(bitfield32_set(NULL, 0U, true) == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_clear(NULL, 0U) == BITFIELD_STATUS_NULL_POINTER);

        assert(bitfield32_set(&v, 32U, true) == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(bitfield32_clear(&v, 32U) == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(v == 0xA5A5A5A5UL);

        assert(bitfield32_set(&v, 31U, false) == BITFIELD_STATUS_OK);
        assert(bitfield32_clear(&v, 0U) == BITFIELD_STATUS_OK);
        assert(v == 0x25A5A5A4UL);

        printf("Bit status (32-bit): PASSED\n");
}

static void
test_range_status(void)
{
        uint16_t v16 = 0x1234U;
        uint32_t v32 = 0x12345678UL;

        assert(bitfield16_set_range(NULL, 0U, 3U, 0xFU)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield16_set_range(&v16, 4U, 3U, 0xFU)
               == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(bitfield16_set_range(&v16, 0U, 16U, 0xFU)
               == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(v16 == 0x1234U);
        assert(bitfield16_set_range(&v16, 0U, 15U, 0xBEEFU)
               == BITFIELD_STATUS_OK);
        assert(v16 == 0xBEEFU);

        assert(bitfield32_set_range(NULL, 0U, 3U, 0xFU)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_set_range(&v32, 4U, 3U, 0xFU)
               == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(bitfield32_set_range(&v32, 0U, 32U, 0xFU)
               == BITFIELD_STATUS_OUT_OF_RANGE);
        assert(v32 == 0x12345678UL);
        assert(bitfield32_set_range(&v32, 0U, 31U, 0xDEADBEEFUL)
               == BITFIELD_STATUS_OK);
        assert(v32 == 0xDEADBEEFUL);

        printf("Range status: PASSED\n");
}

static void
test_pack_status(void)
{
        const uint_least8_t src[5] = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U};
        uint16_t d16[4] = {0xAAAAU, 0xAAAAU, 0xAAAAU, 0xAAAAU};
        uint32_t d32[3] = {0xAAAAAAAAUL, 0xAAAAAAAAUL, 0xAAAAAAAAUL};

        assert(bitfield16_pack(NULL, 5U, d16, 4U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield16_pack(src, 5U, NULL, 4U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_pack(NULL, 5U, d32, 3U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_pack(src, 5U, NULL, 3U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(d16[0] == 0xAAAAU);
        assert(d32[0] == 0xAAAAAAAAUL);

        /* Five bytes need three 16-bit words; two fit. */
        assert(bitfield16_pack(src, 5U, d16, 2U) == BITFIELD_STATUS_TRUNCATED);
        assert((d16[0] == 0x2211U) && (d16[1] == 0x4433U));
        assert(d16[2] == 0xAAAAU);

        /* Exact fit: the final word holds one byte, upper byte zeroed. */
        assert(bitfield16_pack(src, 5U, d16, 3U) == BITFIELD_STATUS_OK);
        assert(d16[2] == 0x0055U);
        assert(d16[3] == 0xAAAAU);

        /* Five bytes need two 32-bit words; one fits. */
        assert(bitfield32_pack(src, 5U, d32, 1U) == BITFIELD_STATUS_TRUNCATED);
        assert(d32[0] == 0x44332211UL);
        assert(d32[1] == 0xAAAAAAAAUL);

        assert(bitfield32_pack(src, 5U, d32, 3U) == BITFIELD_STATUS_OK);
        assert(d32[1] == 0x00000055UL);
        assert(d32[2] == 0xAAAAAAAAUL);

        /* An empty source fits any destination. */
        assert(bitfield16_pack(src, 0U, d16, 0U) == BITFIELD_STATUS_OK);
        assert(bitfield32_pack(src, 0U, d32, 0U) == BITFIELD_STATUS_OK);

        printf("Pack status: PASSED\n");
}

static void
test_unpack_status(void)
{
        const uint16_t s16[2] = {0x2211U, 0x4433U};
        const uint32_t s32[2] = {0x44332211UL, 0x88776655UL};
        uint_least8_t d[9] = {0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU,
                              0xAAU, 0xAAU, 0xAAU, 0xAAU};

        assert(bitfield16_unpack(NULL, 2U, d, 4U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield16_unpack(s16, 2U, NULL, 4U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_unpack(NULL, 2U, d, 8U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(bitfield32_unpack(s32, 2U, NULL, 8U)
               == BITFIELD_STATUS_NULL_POINTER);
        assert(d[0] == 0xAAU);

        /* Two 16-bit words hold four bytes; three fit. */
        assert(bitfield16_unpack(s16, 2U, d, 3U) == BITFIELD_STATUS_TRUNCATED);
        assert((d[0] == 0x11U) && (d[1] == 0x22U) && (d[2] == 0x33U));
        assert(d[3] == 0xAAU);

        /* A larger destination is written only as far as the source. */
        assert(bitfield16_unpack(s16, 2U, d, 5U) == BITFIELD_STATUS_OK);
        assert(d[3] == 0x44U);
        assert(d[4] == 0xAAU);

        /* Two 32-bit words hold eight bytes; five fit. */
        assert(bitfield32_unpack(s32, 2U, d, 5U) == BITFIELD_STATUS_TRUNCATED);
        assert(d[4] == 0x55U);
        assert(d[5] == 0xAAU);

        assert(bitfield32_unpack(s32, 2U, d, 9U) == BITFIELD_STATUS_OK);
        assert((d[5] == 0x66U) && (d[7] == 0x88U));
        assert(d[8] == 0xAAU);

        printf("Unpack status: PASSED\n");
}

int
main(void)
{
        test_bit_status_16();
        test_bit_status_32();
        test_range_status();
        test_pack_status();
        test_unpack_status();

        printf("\nAll status tests PASSED!\n");
        return 0;
}
