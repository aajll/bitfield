# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `bitfield_status_t`, returned by `set`, `clear`, `set_range`, `pack` and `unpack` in both families, which previously returned `void`. Invalid arguments report `BITFIELD_STATUS_NULL_POINTER` or `BITFIELD_STATUS_OUT_OF_RANGE`, and a destination too small for `pack` or `unpack` reports `BITFIELD_STATUS_TRUNCATED`. Calls that discard the result compile unchanged; code that stores these functions in `void`-returning function pointers needs the new type.
- CI cross-builds the library with TI C2000 CGT 22.6.2 and 25.11.1 under the COFF and EABI object formats, with warnings as errors.
- MISRA C:2023 analysis with `misch`, configured by `misra.toml`. CI fails on any finding outside the baseline in `analysis/baseline/`.
- A `clang-format` check through `scripts/format.sh`, which pins the formatter release and runs it with `uvx`.

### Changed

- Status cases are tested in their own executable, and CI gates line and branch coverage at 100 %.
- CI installs Meson, Ninja and gcovr with `uv` instead of `pip`.
- Standardised CI, security policy, contributor guidance, SPDX headers, pkg-config generation, and coverage thresholds with the primitive family baseline.

### Deprecated

- `bitfield_crc16`, in favour of `ucrc_compute()` from the `ucrc` library; with polynomial `0x1021` it equals `ucrc_crc16_ccitt_false`. It stays until a major release.

### Fixed

- Bit positions, counts, lengths and byte arrays use `uint_least8_t` instead of `uint8_t`, so `bitfield.h` compiles where `CHAR_BIT == 16`. Byte-addressed targets see the same types.
- `bitfield_version.h` now defines `BITFIELD_VERSION` as a single string literal and `BITFIELD_VERSION_MAJOR`, `_MINOR` and `_PATCH` as integers, so the macros compile when used.

## [1.3.3] - 2026-03-16

### Changed

- Improved support for separate 16-bit and 32-bit bitfield API families while preserving portability across embedded targets with different native word sizes.

## [1.1.3] - 2026-03-11

### Changed

- Moved build configuration templates into `config/`, enabled automatic unit-test builds, and cleaned up Meson build formatting.
