/**
 * SPDX-License-Identifier: MIT
 *
 * @file: bitfield.h
 *
 * @brief
 *    Portable bit manipulation library for embedded systems.
 *
 * This library provides deterministic bit manipulation operations
 * suitable for safety-critical applications. Two typed API families
 * are provided: @c bitfield16_* for 16-bit fields and @c bitfield32_*
 * for 32-bit fields covering the full range of word sizes encountered
 * on 8/16/32-bit embedded targets and 64-bit hosts alike.
 *
 * No compile-time configuration is required. Both families are always
 * available and may be mixed freely within the same translation unit,
 * which is useful on targets where 16-bit and 32-bit peripheral registers
 * coexist in the same driver.
 *
 * Bit positions, counts and lengths are uint_least8_t, and byte arrays are
 * arrays of uint_least8_t holding one octet per element. Where CHAR_BIT is
 * 16 only the low 8 bits of each element are read or written.
 */

#ifndef BITFIELD_H
#define BITFIELD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Result of a function that modifies a field or buffer.
 *
 * @details
 * - ::BITFIELD_STATUS_OK            the operation completed.
 * - ::BITFIELD_STATUS_NULL_POINTER  a pointer argument was NULL; nothing
 *   was written.
 * - ::BITFIELD_STATUS_OUT_OF_RANGE  a bit position or range lies outside
 *   the field; nothing was written.
 * - ::BITFIELD_STATUS_TRUNCATED     the destination was too small; it was
 *   filled as far as it goes and the rest of the source was ignored.
 */
typedef enum {
        BITFIELD_STATUS_OK = 0,
        BITFIELD_STATUS_NULL_POINTER = 1,
        BITFIELD_STATUS_OUT_OF_RANGE = 2,
        BITFIELD_STATUS_TRUNCATED = 3
} bitfield_status_t;

/** @defgroup bitfield16 16-bit Bitfield Operations
 * @brief Operations on a single @c uint16_t field
 * @{
 */

/** Set a single bit in a 16-bit field to the specified value
 *
 * @param data    Pointer to the 16-bit field
 * @param bit_pos Bit position (0 = LSB, 15 = MSB)
 * @param value   Boolean value to set (true = 1, false = 0)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p bit_pos >= 16.
 */
bitfield_status_t bitfield16_set(uint16_t *data, uint_least8_t bit_pos,
                                 bool value);

/** Get the value of a single bit in a 16-bit field
 *
 * @param data    Pointer to the 16-bit field
 * @param bit_pos Bit position (0 = LSB, 15 = MSB)
 * @return Boolean value of the bit (true = 1, false = 0)
 *
 * If @p data is NULL or @p bit_pos >= 16, this function returns false.
 */
bool bitfield16_get(const uint16_t *data, uint_least8_t bit_pos);

/** Clear a single bit in a 16-bit field (set to 0)
 *
 * @param data    Pointer to the 16-bit field
 * @param bit_pos Bit position (0 = LSB, 15 = MSB)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p bit_pos >= 16.
 */
bitfield_status_t bitfield16_clear(uint16_t *data, uint_least8_t bit_pos);

/** Set a range of bits in a 16-bit field
 *
 * @param data  Pointer to the 16-bit field
 * @param start Start bit position (0 = LSB, inclusive)
 * @param end   End bit position (inclusive, must be < 16)
 * @param value Value to write (LSB of value maps to @p start)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p start > @p end
 *         or @p end >= 16.
 *
 * Bits outside the range [@p start, @p end] are preserved.
 */
bitfield_status_t bitfield16_set_range(uint16_t *data, uint_least8_t start,
                                       uint_least8_t end, uint32_t value);

/** Get a range of bits from a 16-bit field
 *
 * @param data  Pointer to the 16-bit field
 * @param start Start bit position (0 = LSB, inclusive)
 * @param end   End bit position (inclusive, must be < 16)
 * @return Extracted bit range, right-aligned (bit @p start maps to bit 0)
 *
 * If @p data is NULL, @p start > @p end, or @p end >= 16, returns 0.
 */
uint32_t bitfield16_get_range(const uint16_t *data, uint_least8_t start,
                              uint_least8_t end);

/** Count the number of set bits (1s) in a 16-bit field
 *
 * @param data     Pointer to the 16-bit field
 * @param num_bits Number of bits to examine, starting from LSB (max 16)
 * @return Number of set bits
 *
 * If @p data is NULL or @p num_bits > 16, returns 0.
 */
uint_least8_t bitfield16_count_set(const uint16_t *data,
                                   uint_least8_t num_bits);

/** Count the number of unset bits (0s) in a 16-bit field
 *
 * @param data     Pointer to the 16-bit field
 * @param num_bits Number of bits to examine, starting from LSB (max 16)
 * @return Number of unset bits
 *
 * If @p data is NULL or @p num_bits > 16, returns 0.
 */
uint_least8_t bitfield16_count_unset(const uint16_t *data,
                                     uint_least8_t num_bits);

/** Pack a byte array into an array of 16-bit words (little-endian)
 *
 * @param src     Source byte array
 * @param src_len Length of source array in bytes
 * @param dst     Destination array of 16-bit words
 * @param dst_len Length of destination array in words
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p src
 *         or @p dst is NULL, or ::BITFIELD_STATUS_TRUNCATED if @p dst
 *         holds fewer words than the source needs.
 *
 * Bytes are packed two at a time into each destination word, LSB first;
 * a final partial word has its unused upper bytes zeroed. On truncation
 * all @p dst_len words are written.
 */
bitfield_status_t bitfield16_pack(const uint_least8_t *src,
                                  uint_least8_t src_len, uint16_t *dst,
                                  uint_least8_t dst_len);

/** Unpack an array of 16-bit words into a byte array (little-endian)
 *
 * @param src     Source array of 16-bit words
 * @param src_len Length of source array in words
 * @param dst     Destination byte array
 * @param dst_len Length of destination array in bytes
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p src
 *         or @p dst is NULL, or ::BITFIELD_STATUS_TRUNCATED if @p dst
 *         holds fewer bytes than the source words contain.
 *
 * Bytes are produced LSB first. On truncation all @p dst_len bytes are
 * written; when the source is shorter, bytes past it are left unchanged.
 */
bitfield_status_t bitfield16_unpack(const uint16_t *src, uint_least8_t src_len,
                                    uint_least8_t *dst, uint_least8_t dst_len);

/** @} */

/** @defgroup bitfield32 32-bit Bitfield Operations
 * @brief Operations on a single @c uint32_t field
 * @{
 */

/** Set a single bit in a 32-bit field to the specified value
 *
 * @param data    Pointer to the 32-bit field
 * @param bit_pos Bit position (0 = LSB, 31 = MSB)
 * @param value   Boolean value to set (true = 1, false = 0)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p bit_pos >= 32.
 */
bitfield_status_t bitfield32_set(uint32_t *data, uint_least8_t bit_pos,
                                 bool value);

/** Get the value of a single bit in a 32-bit field
 *
 * @param data    Pointer to the 32-bit field
 * @param bit_pos Bit position (0 = LSB, 31 = MSB)
 * @return Boolean value of the bit (true = 1, false = 0)
 *
 * If @p data is NULL or @p bit_pos >= 32, this function returns false.
 */
bool bitfield32_get(const uint32_t *data, uint_least8_t bit_pos);

/** Clear a single bit in a 32-bit field (set to 0)
 *
 * @param data    Pointer to the 32-bit field
 * @param bit_pos Bit position (0 = LSB, 31 = MSB)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p bit_pos >= 32.
 */
bitfield_status_t bitfield32_clear(uint32_t *data, uint_least8_t bit_pos);

/** Set a range of bits in a 32-bit field
 *
 * @param data  Pointer to the 32-bit field
 * @param start Start bit position (0 = LSB, inclusive)
 * @param end   End bit position (inclusive, must be < 32)
 * @param value Value to write (LSB of value maps to @p start)
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p data
 *         is NULL, or ::BITFIELD_STATUS_OUT_OF_RANGE if @p start > @p end
 *         or @p end >= 32.
 *
 * Bits outside the range [@p start, @p end] are preserved.
 */
bitfield_status_t bitfield32_set_range(uint32_t *data, uint_least8_t start,
                                       uint_least8_t end, uint32_t value);

/** Get a range of bits from a 32-bit field
 *
 * @param data  Pointer to the 32-bit field
 * @param start Start bit position (0 = LSB, inclusive)
 * @param end   End bit position (inclusive, must be < 32)
 * @return Extracted bit range, right-aligned (bit @p start maps to bit 0)
 *
 * If @p data is NULL, @p start > @p end, or @p end >= 32, returns 0.
 */
uint32_t bitfield32_get_range(const uint32_t *data, uint_least8_t start,
                              uint_least8_t end);

/** Count the number of set bits (1s) in a 32-bit field
 *
 * @param data     Pointer to the 32-bit field
 * @param num_bits Number of bits to examine, starting from LSB (max 32)
 * @return Number of set bits
 *
 * If @p data is NULL or @p num_bits > 32, returns 0.
 */
uint_least8_t bitfield32_count_set(const uint32_t *data,
                                   uint_least8_t num_bits);

/** Count the number of unset bits (0s) in a 32-bit field
 *
 * @param data     Pointer to the 32-bit field
 * @param num_bits Number of bits to examine, starting from LSB (max 32)
 * @return Number of unset bits
 *
 * If @p data is NULL or @p num_bits > 32, returns 0.
 */
uint_least8_t bitfield32_count_unset(const uint32_t *data,
                                     uint_least8_t num_bits);

/** Pack a byte array into an array of 32-bit words (little-endian)
 *
 * @param src     Source byte array
 * @param src_len Length of source array in bytes
 * @param dst     Destination array of 32-bit words
 * @param dst_len Length of destination array in words
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p src
 *         or @p dst is NULL, or ::BITFIELD_STATUS_TRUNCATED if @p dst
 *         holds fewer words than the source needs.
 *
 * Bytes are packed four at a time into each destination word, LSB first;
 * a final partial word has its unused upper bytes zeroed. On truncation
 * all @p dst_len words are written.
 */
bitfield_status_t bitfield32_pack(const uint_least8_t *src,
                                  uint_least8_t src_len, uint32_t *dst,
                                  uint_least8_t dst_len);

/** Unpack an array of 32-bit words into a byte array (little-endian)
 *
 * @param src     Source array of 32-bit words
 * @param src_len Length of source array in words
 * @param dst     Destination byte array
 * @param dst_len Length of destination array in bytes
 *
 * @return ::BITFIELD_STATUS_OK, ::BITFIELD_STATUS_NULL_POINTER if @p src
 *         or @p dst is NULL, or ::BITFIELD_STATUS_TRUNCATED if @p dst
 *         holds fewer bytes than the source words contain.
 *
 * Bytes are produced LSB first. On truncation all @p dst_len bytes are
 * written; when the source is shorter, bytes past it are left unchanged.
 */
bitfield_status_t bitfield32_unpack(const uint32_t *src, uint_least8_t src_len,
                                    uint_least8_t *dst, uint_least8_t dst_len);

/** @} */

/** @defgroup crc CRC Calculations
 * @brief Cyclic Redundancy Check functions (field-width independent)
 * @{
 */

/** Calculate 16-bit CRC using specified polynomial
 *
 * @param data       Data to calculate CRC for
 * @param length     Length of data in bytes
 * @param polynomial CRC polynomial (e.g., 0x8005 for CRC-16)
 * @return Calculated CRC value
 *
 * If @p data is NULL, this function returns 0.
 */
uint16_t bitfield_crc16(const uint_least8_t *data, uint16_t length,
                        uint16_t polynomial);

/** @} */

#endif /* BITFIELD_H */
