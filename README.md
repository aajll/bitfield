# bitfield

[![CI](https://github.com/aajll/bitfield/actions/workflows/ci.yml/badge.svg)](https://github.com/aajll/bitfield/actions/workflows/ci.yml)

A portable bit manipulation C library for safety-critical embedded systems.

## Features

- **Universal field widths** - Explicit `bitfield16_*` and `bitfield32_*` API families; both always available, no configuration required
- **No dynamic memory** - Fixed-size operations, no `malloc` / `free`
- **Deterministic WCET** - All operations have bounded execution time
- **Mixed-width friendly** - 16-bit and 32-bit families freely mixable in the same translation unit (e.g. TI C2000 drivers with both 16-bit control registers and 32-bit data registers)
- **Bit-level operations** - Individual bit access and bitfield manipulation
- **Packing/unpacking** - Compact data representation for protocols
- **CRC calculations** - Data integrity verification

## Installation

### Copy-in (recommended for embedded targets)

Copy two files into your project tree:

```
include/bitfield.h
src/bitfield.c
```

Then include the header:

```c
#include "bitfield.h"
```

### Meson subproject

Add this repo as a wrap dependency or subproject:

```meson
bitfield_dep = dependency('bitfield', fallback : ['bitfield', 'bitfield_dep'])
```

## Quick Start

```c
#include "bitfield.h"

int main(void)
{
    /* --- 16-bit field (e.g. a TI C2000 control register) --- */
    uint16_t ctrl = 0U;

    bitfield16_set(&ctrl, 4U, true);              /* set enable bit */
    bitfield16_set_range(&ctrl, 0U, 1U, 0b10U);   /* mode = 2 */
    bool enabled = bitfield16_get(&ctrl, 4U);
    uint32_t mode = bitfield16_get_range(&ctrl, 0U, 1U);

    /* --- 32-bit field (e.g. a peripheral period register) --- */
    uint32_t period = 0U;

    bitfield32_set_range(&period, 0U, 31U, 100000U);
    uint32_t val = bitfield32_get_range(&period, 0U, 31U);

    /* --- Count bits --- */
    uint_least8_t n = bitfield32_count_set(&period, 32U);

    /* --- Pack bytes into 32-bit words for a protocol frame --- */
    uint_least8_t payload[4] = {0x11U, 0x22U, 0x33U, 0x44U};
    uint32_t frame[1] = {0U};
    bitfield32_pack(payload, 4U, frame, 1U);  /* frame[0] == 0x44332211 */

    return 0;
}
```

## API Reference

Both families share identical semantics; only the field type and valid bit
position range differ.

### 16-bit family (`uint16_t` fields, bit positions 0–15)

```c
void     bitfield16_set(uint16_t *data, uint_least8_t bit_pos, bool value);
bool     bitfield16_get(const uint16_t *data, uint_least8_t bit_pos);
void     bitfield16_clear(uint16_t *data, uint_least8_t bit_pos);

void     bitfield16_set_range(uint16_t *data, uint_least8_t start, uint_least8_t end, uint32_t value);
uint32_t bitfield16_get_range(const uint16_t *data, uint_least8_t start, uint_least8_t end);

uint_least8_t  bitfield16_count_set(const uint16_t *data, uint_least8_t num_bits);
uint_least8_t  bitfield16_count_unset(const uint16_t *data, uint_least8_t num_bits);

void     bitfield16_pack(const uint_least8_t *src, uint_least8_t src_len, uint16_t *dst, uint_least8_t dst_len);
void     bitfield16_unpack(const uint16_t *src, uint_least8_t src_len, uint_least8_t *dst, uint_least8_t dst_len);
```

### 32-bit family (`uint32_t` fields, bit positions 0–31)

```c
void     bitfield32_set(uint32_t *data, uint_least8_t bit_pos, bool value);
bool     bitfield32_get(const uint32_t *data, uint_least8_t bit_pos);
void     bitfield32_clear(uint32_t *data, uint_least8_t bit_pos);

void     bitfield32_set_range(uint32_t *data, uint_least8_t start, uint_least8_t end, uint32_t value);
uint32_t bitfield32_get_range(const uint32_t *data, uint_least8_t start, uint_least8_t end);

uint_least8_t  bitfield32_count_set(const uint32_t *data, uint_least8_t num_bits);
uint_least8_t  bitfield32_count_unset(const uint32_t *data, uint_least8_t num_bits);

void     bitfield32_pack(const uint_least8_t *src, uint_least8_t src_len, uint32_t *dst, uint_least8_t dst_len);
void     bitfield32_unpack(const uint32_t *src, uint_least8_t src_len, uint_least8_t *dst, uint_least8_t dst_len);
```

### CRC (field-width independent)

```c
uint16_t bitfield_crc16(const uint_least8_t *data, uint16_t length, uint16_t polynomial);
```

## Input Validation

All public APIs validate pointers and bit indices/ranges at the API boundary.

- Passing a `NULL` pointer is safe; mutating functions become no-ops.
- Out-of-range bit positions/ranges are ignored.
- Value-returning functions return a safe default (`false` or `0U`) on invalid input.

## Building

```sh
# Library only (release)
meson setup build --buildtype=release
meson compile -C build

# With unit tests
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build
```

## Use Cases

1. **Protocol Encoding** - Compact data for CAN, I2C, or custom protocols
2. **Register Access** - Safe manipulation of peripheral registers on any word-width target
3. **State Encoding** - Compact state representation in state machines
4. **Data Compression** - Space-efficient storage for constrained systems
5. **Bitmask Operations** - Event flag management and bitmask testing
6. **Error Detection** - CRC for data integrity verification

## Notes

| Topic | Note |
|---|---|
| **Memory** | All operations use static memory; no dynamic allocation |
| **Thread safety** | Not thread-safe; protect with external mutex if needed |
| **WCET** | All operations have deterministic worst-case execution time |
| **Platforms** | No platform detection or configuration required. Positions, counts and lengths are `uint_least8_t`, and byte arrays hold one octet per `uint_least8_t`, so the library builds where `CHAR_BIT == 16`; there only the low 8 bits of each element are used. CI cross-builds with TI C2000 CGT 22.6.2 and 25.11.1 (COFF and EABI) |
| **Version Header** | The module version header is auto-generated by the Meson build and placed in the output build folder |
