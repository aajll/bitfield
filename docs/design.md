# Design

`bitfield` is a C11 library of checked operations on words held by the caller: single bits, inclusive bit ranges, bit counts, and conversion between octet buffers and 16-bit or 32-bit words. It owns no memory and no state; every function works on the pointers it is given.

## Goals and non-goals

Goals:

- Bit and range access that rejects an invalid position instead of shifting past the word.
- The same semantics for 16-bit and 32-bit words, with both families always available.
- Bounded execution time, no heap, and no global state.
- Correct behaviour where `CHAR_BIT` is 16.

Non-goals:

- Arbitrary-length bit arrays. A word is 16 or 32 bits; the `bitmap` library covers longer sets.
- Big-endian packing, or bit order other than least significant first.
- Atomic or interrupt-safe register access. The caller owns concurrency.
- CRC computation. `bitfield_crc16` remains for existing callers only; the `ucrc` library replaces it.

## Bit numbering and ranges

Bit 0 is the least significant bit of the word. A range is the inclusive interval `[start, end]`, which matches how datasheets describe register fields: bits 8 to 11 is a four-bit field. A range is valid when `start <= end` and `end` is below the word width. Every check happens before any write, so an invalid call leaves the word untouched.

### Masks in a wider type

A range of width `w` at `start` needs the mask `((1 << w) - 1) << start`. For a full-width range, `w` equals the word width, and shifting a word by its own width is undefined in C. The 16-bit family therefore builds masks in `uint32_t` and the 32-bit family in `uint64_t`, so the widest shift is always smaller than the accumulator. The result is narrowed back to the word type after masking. This removes the special case for a full-width range and keeps every shift defined.

`set_range` masks `value` into the range, so bits of `value` beyond the range width are discarded rather than spilling into neighbouring fields.

## Counting

`count_set` and `count_unset` examine the low `num_bits` bits one at a time. The loop runs at most 32 times and has no data-dependent exit, so its time depends only on `num_bits`. A table or compiler popcount builtin would be faster on some targets but would add either memory or a compiler dependency.

## Packing

`pack` builds each word from consecutive octets, least significant byte first, so the first octet in a buffer is the low byte of the first word on every host, whatever its own endianness. A final partial word is zero-filled above the last octet. `unpack` is the exact inverse for whole words.

The status reports what happened to the destination:

- `BITFIELD_STATUS_NULL_POINTER`: nothing written.
- `BITFIELD_STATUS_TRUNCATED`: every destination element written, the remaining source ignored.
- `BITFIELD_STATUS_OK`: the whole source converted. `unpack` leaves destination elements past the source untouched.

Both loops are bounded by the smaller of the source and destination lengths, counted in destination elements. Lengths are widened to `uint_fast16_t` before the word-to-octet multiplication, so where `uint_least8_t` is 8 bits wide the product cannot wrap.

## Octets on wide-byte targets

Positions, counts and lengths are `uint_least8_t`, and byte buffers are arrays of `uint_least8_t`, not `uint8_t`, which does not exist where `CHAR_BIT` is 16. Each element holds one octet. `pack` reads only the low 8 bits of each element and `unpack` writes values below 256, so a buffer means the same thing on an 8-bit-byte host and a 16-bit-MAU target. A buffer that crosses a wire or a file format between those targets carries one octet per element on both.

## Status and defaults

Writers return `bitfield_status_t`, an enum whose first value, `BITFIELD_STATUS_OK`, is zero. Readers return plain values and use `false` or `0` for invalid input, because a status return would cost every caller an out-parameter for the common, valid case. A caller that must tell an invalid position from a zero bit checks the position first.

## Portability

- C11 with `<stdint.h>`, `<stdbool.h>`, and `<stddef.h>`. No other dependencies.
- No `uint8_t`, so the library builds where `CHAR_BIT` is 16.
- Shifts are performed in an unsigned type at least one bit wider than the shift count, so none is undefined on any `int` width.
- The library needs `uint64_t` for the 32-bit family's masks.

## Verification

The unit suite covers both families: every bit position, valid, invalid, and adjacent ranges including the full-width range, counts at zero and full width, packing of whole and partial words, round trips, and the deprecated CRC against its CRC-16/CCITT-FALSE check value. A separate executable checks every status: each rejection leaves the destination unchanged, and truncation fills the destination as far as it goes. CI runs the suite under AddressSanitizer and UndefinedBehaviorSanitizer with warnings as errors, and `src/` and `include/` reach 100 percent line and branch coverage. A documentation test compiles every C example in the README and `docs/` against the header.

CI compiles the library with warnings as errors under TI C2000 Code Generation Tools 22.6.2.LTS and 25.11.1.LTS in C11 C28x mode, under both the COFF and EABI ABIs. This validates the source dialect and type assumptions only. Linking, target execution, and final map-file footprint remain consumer-owned checks.

MISRA C:2023 analysis with `misch` runs against a recorded baseline in `analysis/baseline/`, and CI fails on any finding outside it. The version macros in `bitfield.h` carry an inline deviation of Rule 2.5 (unused macro), because they exist for consumers and the library does not use them.
