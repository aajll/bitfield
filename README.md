# bitfield

[![CI](https://github.com/aajll/bitfield/actions/workflows/ci.yml/badge.svg)](https://github.com/aajll/bitfield/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**Checked bit, range, count and pack operations on 16-bit and 32-bit words, for embedded C.**

## Quick start

Copy `include/bitfield.h` and `src/bitfield.c` into your project. For a Meson subproject or an installed `pkg-config` package, see the [integration guide](docs/integration.md).

```c
#include "bitfield.h"

static uint16_t ctrl;   /* a 16-bit control register image */
static uint32_t frame[1];

bool
configure(void)
{
        bool ok = true;

        ok = ok && (bitfield16_set(&ctrl, 4U, true) == BITFIELD_STATUS_OK);
        ok = ok && (bitfield16_set_range(&ctrl, 0U, 1U, 0x2U) == BITFIELD_STATUS_OK);

        const uint_least8_t payload[4] = {0x11U, 0x22U, 0x33U, 0x44U};
        ok = ok && (bitfield32_pack(payload, 4U, frame, 1U) == BITFIELD_STATUS_OK);
        /* frame[0] == 0x44332211 */

        return ok && (bitfield16_get_range(&ctrl, 0U, 1U) == 0x2U);
}
```

## Description

`bitfield` reads and writes single bits and inclusive bit ranges in a caller's word, counts set and clear bits, and packs octets into words and back, little-endian. Every function checks its pointers and positions, so a bad index reports an error instead of shifting past the word.

- `bitfield16_*` and `bitfield32_*` families with the same semantics, always both available and freely mixed
- Functions that write return `bitfield_status_t`; queries return `false` or `0` on invalid input
- Invalid input writes nothing, and a short destination is filled as far as it goes and reported as truncated
- Bounded loops, no heap, and no global state
- Builds where `CHAR_BIT` is 16, using one octet per `uint_least8_t`
- 100% line and branch coverage, sanitiser-clean tests, and a MISRA C:2023 baseline from [`misch`](https://pypi.org/project/misch/)

The functions are not thread-safe: each write is a read-modify-write of the caller's word, so callers serialise concurrent access to one word.

## Validated toolchains

| Toolchain               | Target                        | Validation                         |
| ----------------------- | ----------------------------- | ---------------------------------- |
| GCC, Clang              | Linux hosts                   | Unit and sanitiser tests           |
| DSP vendor C11 compiler | 16-bit MAU (`CHAR_BIT == 16`) | Library compile, two object formats |

Target linking and execution belong to the consuming firmware. See [cross builds](docs/integration.md#cross-builds).

## Documentation

- [Integration guide](docs/integration.md) — copy-in, Meson subproject, installed package, and cross builds
- [API reference](docs/api.md) — every function, with its rules
- [Design](docs/design.md) — bit numbering, range masks, packing, octets on wide-byte targets, and verification
- [Changelog](CHANGELOG.md) — release history

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before you open a pull request. Report security issues as described in [SECURITY.md](.github/SECURITY.md).

## License

MIT. See [LICENSE](LICENSE).
