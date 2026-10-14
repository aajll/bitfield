# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- CI cross-builds the library with TI C2000 CGT 22.6.2 and 25.11.1 under the COFF and EABI object formats, with warnings as errors.

### Changed

- Standardised CI, security policy, contributor guidance, SPDX headers, pkg-config generation, and coverage thresholds with the primitive family baseline.

### Fixed

- Bit positions, counts, lengths and byte arrays use `uint_least8_t` instead of `uint8_t`, so `bitfield.h` compiles where `CHAR_BIT == 16`. Byte-addressed targets see the same types.
- `bitfield_version.h` now defines `BITFIELD_VERSION` as a single string literal and `BITFIELD_VERSION_MAJOR`, `_MINOR` and `_PATCH` as integers, so the macros compile when used.

## [1.3.3] - 2026-03-16

### Changed

- Improved support for separate 16-bit and 32-bit bitfield API families while preserving portability across embedded targets with different native word sizes.

## [1.1.3] - 2026-03-11

### Changed

- Moved build configuration templates into `config/`, enabled automatic unit-test builds, and cleaned up Meson build formatting.
