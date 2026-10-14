# API reference

Every declaration lives in [`include/bitfield.h`](../include/bitfield.h), which carries the full Doxygen contract. The reasons behind these rules are in [design.md](design.md).

## Conventions

- Two families, `bitfield16_*` on `uint16_t` and `bitfield32_*` on `uint32_t`, with identical semantics. Bit positions run from 0, the least significant bit, to 15 or 31.
- Ranges are inclusive: `start` and `end` both belong to the range, so `start == end` is a one-bit range.
- Positions, counts and buffer lengths are `uint_least8_t`. Byte buffers hold one octet per `uint_least8_t` element.
- Functions that write return `bitfield_status_t`. Functions that read return `false` or `0` on a `NULL` pointer or an invalid position, which is indistinguishable from a real zero; validate positions first when that matters.
- The library keeps no state. Each write is a read-modify-write of the caller's word, so concurrent access to one word needs the caller's own serialisation.

## Status

<!-- snippet: skip -->
```c
typedef enum {
        BITFIELD_STATUS_OK = 0,
        BITFIELD_STATUS_NULL_POINTER = 1,
        BITFIELD_STATUS_OUT_OF_RANGE = 2,
        BITFIELD_STATUS_TRUNCATED = 3
} bitfield_status_t;
```

`BITFIELD_STATUS_NULL_POINTER` and `BITFIELD_STATUS_OUT_OF_RANGE` mean nothing was written. `BITFIELD_STATUS_TRUNCATED`, from `pack` and `unpack` only, means the destination was too small: it was filled as far as it goes and the rest of the source was ignored. A call that discards the result still compiles.

## Single bits

<!-- snippet: skip -->
```c
bitfield_status_t bitfield16_set(uint16_t *data, uint_least8_t bit_pos, bool value);
bitfield_status_t bitfield16_clear(uint16_t *data, uint_least8_t bit_pos);
bool bitfield16_get(const uint16_t *data, uint_least8_t bit_pos);

bitfield_status_t bitfield32_set(uint32_t *data, uint_least8_t bit_pos, bool value);
bitfield_status_t bitfield32_clear(uint32_t *data, uint_least8_t bit_pos);
bool bitfield32_get(const uint32_t *data, uint_least8_t bit_pos);
```

`set` writes `value` to bit `bit_pos` and `clear` writes zero; every other bit is preserved. Both return `BITFIELD_STATUS_OUT_OF_RANGE` when `bit_pos` is at or past the word width. `get` returns the bit, or `false` for a `NULL` pointer or an invalid position.

## Ranges

<!-- snippet: skip -->
```c
bitfield_status_t bitfield16_set_range(uint16_t *data, uint_least8_t start, uint_least8_t end, uint32_t value);
uint32_t bitfield16_get_range(const uint16_t *data, uint_least8_t start, uint_least8_t end);

bitfield_status_t bitfield32_set_range(uint32_t *data, uint_least8_t start, uint_least8_t end, uint32_t value);
uint32_t bitfield32_get_range(const uint32_t *data, uint_least8_t start, uint_least8_t end);
```

`set_range` writes the low `end - start + 1` bits of `value` into bits `start` to `end`, with the least significant bit of `value` at `start`. Higher bits of `value` are discarded, and bits outside the range are preserved. `get_range` returns bits `start` to `end` shifted down so that `start` lands at bit 0. A range is valid when `start <= end` and `end` is below the word width; the full word, `0` to `31` or `0` to `15`, is a valid range. An invalid range makes `set_range` return `BITFIELD_STATUS_OUT_OF_RANGE` and `get_range` return `0`.

<!-- snippet: body -->
```c
uint32_t reg = 0xFFFFFFFFU;

(void)bitfield32_set_range(&reg, 8U, 11U, 0x5U); /* reg == 0xFFFFF5FF */
uint32_t nibble = bitfield32_get_range(&reg, 8U, 11U); /* 0x5 */
(void)nibble;
```

## Counting

<!-- snippet: skip -->
```c
uint_least8_t bitfield16_count_set(const uint16_t *data, uint_least8_t num_bits);
uint_least8_t bitfield16_count_unset(const uint16_t *data, uint_least8_t num_bits);

uint_least8_t bitfield32_count_set(const uint32_t *data, uint_least8_t num_bits);
uint_least8_t bitfield32_count_unset(const uint32_t *data, uint_least8_t num_bits);
```

`count_set` returns the number of one bits, and `count_unset` the number of zero bits, among the low `num_bits` bits of the word. `num_bits` may be zero, which counts nothing, and at most the word width. A `NULL` pointer or a larger `num_bits` returns `0`.

## Packing

<!-- snippet: skip -->
```c
bitfield_status_t bitfield16_pack(const uint_least8_t *src, uint_least8_t src_len, uint16_t *dst, uint_least8_t dst_len);
bitfield_status_t bitfield16_unpack(const uint16_t *src, uint_least8_t src_len, uint_least8_t *dst, uint_least8_t dst_len);

bitfield_status_t bitfield32_pack(const uint_least8_t *src, uint_least8_t src_len, uint32_t *dst, uint_least8_t dst_len);
bitfield_status_t bitfield32_unpack(const uint32_t *src, uint_least8_t src_len, uint_least8_t *dst, uint_least8_t dst_len);
```

`pack` combines `src_len` octets into words, two or four per word, with the first octet in the least significant byte. A final partial word has its unused upper bytes zeroed. `unpack` reverses it, writing `src_len` words out as octets, least significant byte first. Lengths count elements: octets for byte buffers and words for word buffers.

When the destination is too small, both fill all `dst_len` elements and return `BITFIELD_STATUS_TRUNCATED`. When `unpack` has more room than the source needs, elements past the source are left unchanged. Either pointer being `NULL` returns `BITFIELD_STATUS_NULL_POINTER`. Only the low 8 bits of each source octet are read.

<!-- snippet: body -->
```c
const uint_least8_t bytes[3] = {0xAAU, 0xBBU, 0xCCU};
uint16_t words[2];
uint_least8_t back[4] = {0U};

(void)bitfield16_pack(bytes, 3U, words, 2U);   /* {0xBBAA, 0x00CC} */
(void)bitfield16_unpack(words, 2U, back, 4U);  /* {0xAA, 0xBB, 0xCC, 0x00} */
```

## CRC (deprecated)

<!-- snippet: skip -->
```c
uint16_t bitfield_crc16(const uint_least8_t *data, uint16_t length, uint16_t polynomial);
```

`bitfield_crc16` is deprecated in favour of `ucrc_compute()` from the `ucrc` library, and stays until a major release. It starts the register at `0xFFFF`, reflects neither input nor output, and applies no final XOR, so with polynomial `0x1021` it computes CRC-16/CCITT-FALSE, check value `0x29B1`. It returns `0` for a `NULL` pointer.

<!-- snippet: skip -->
```c
/* Before */
uint16_t crc = bitfield_crc16(data, len, 0x1021U);

/* After */
uint16_t crc = (uint16_t)ucrc_compute(&ucrc_crc16_ccitt_false, data, len);
```

For another polynomial, define a `ucrc_model_t` with that `poly`, `init` `0xFFFF`, `xorout` `0`, `width` 16, no reflection and a `NULL` table.
