# Contributing to bitfield

`bitfield` is a small C11 library of checked bit, range, count and pack operations on 16-bit and 32-bit words. Two contracts carry most of the design weight: an invalid argument never writes and never shifts past the word, and the 16-bit and 32-bit families behave identically. Keep both in mind when you change anything.

## Getting started

Install a C11 compiler and install the Python tools with [uv](https://docs.astral.sh/uv/), as CI does. The MISRA analysis also needs a current `cppcheck`; CI installs it from Homebrew because the 2.13 release in Ubuntu 24.04 reports different findings. The same commands CI runs, locally:

```sh
# Python tools
uv tool install meson
uv tool install ninja
uv tool install gcovr
uv tool install misch

# Configure with tests and sanitisers (CI default)
meson setup build --buildtype=debug -Dbuild_tests=true -Db_sanitize=address,undefined
meson compile -C build
meson test -C build --verbose

# Coverage (CI gate is 100% line and 100% branch for src/ and include/)
meson setup build_cov --buildtype=debug -Dbuild_tests=true -Db_coverage=true
meson compile -C build_cov && meson test -C build_cov
gcovr --root . --filter 'src/' --filter 'include/' --print-summary

# MISRA C:2023 against the recorded baseline (reads build/compile_commands.json)
misch run --baseline

# Formatting (pinned clang-format through uvx)
scripts/format.sh --fix
```

## Source style

- Formatting is mandatory. Run `scripts/format.sh --fix` before submitting, or `scripts/format.sh` to check without writing, as CI does. The script pins the clang-format release and runs it through `uvx`, so an editor formatter gives the same result only when it runs the same release.
- 8-space indent, Linux brace style, 80-column limit. Match the existing conventions and do not reformat unrelated code.
- The Meson build system is the single source of truth. Update `meson.build` and `tests/meson.build` when you add or remove files.
- No CMake, no Make, and no other build systems.

## C language rules

- C11 only.
- Use fixed-width types from `<stdint.h>` and `<stdbool.h>`. Do not use `uint8_t`: positions, counts, lengths and byte buffers are `uint_least8_t`, so the library builds where `CHAR_BIT` is 16.
- No heap allocation (`malloc`, `free`, VLAs), no recursion, and no global state. Every loop is bounded by a word width or a buffer length.
- Validate pointers and positions at every public function before any write. Writers return `bitfield_status_t`; readers return `false` or `0`.
- Keep the 16-bit and 32-bit families consistent. A change to one family goes to the other in the same pull request.

## MISRA C:2023

The library is analysed with `misch` (cppcheck-backed MISRA C:2023 analysis), configured by `misra.toml`, over `include/` and `src/`. It reads the compilation database from a Meson `build/` directory, so configure one first. The baseline in `analysis/baseline/` records the existing findings, and CI fails on any finding that is not in it.

The baseline is recorded without rule texts, and the texts change the messages a finding is matched on. If you have `MISRA_RULE_TEXTS` set, unset it for `misch run --baseline` as well, as CI does.

A change must not add findings. If it removes one, regenerate the baseline with `MISRA_RULE_TEXTS` unset:

```sh
env -u MISRA_RULE_TEXTS misch baseline
```

Full tool-driven compliance beyond this analysis needs a certified static analyser, which this repository does not vendor.

## Tests and coverage

- Add a test for every bug fix and every new behaviour, in both families.
- `tests/test_bitfield.c` covers behaviour and `tests/test_status.c` covers every status return. Register new files in `tests/meson.build`.
- CI enforces 100 percent line and branch coverage of `src/` and `include/`.
- The `bitfield docs examples` test compiles every C code block in `README.md`, `CONTRIBUTING.md`, and `docs/` against the public header, through `tests/docs/check_snippets.py`. A block compiles at file scope by default. On the line directly above the fence, put `<!-- snippet: body -->` for a block of statements, or `<!-- snippet: skip -->` for pseudo-code or a copy of a header definition.

## API stability

- The public function signatures in `bitfield.h` are stable across a major version. Breaking changes go in a new major release with a note in `CHANGELOG.md`.
- `bitfield_crc16` is deprecated and stays until the next major release. Do not extend it; CRC work belongs in `ucrc`.

## Documentation

| File | Purpose |
| --- | --- |
| `README.md` | Landing page: quick start, description, validated toolchains, and links. Keep it short. |
| `docs/integration.md` | Installation, building, and cross builds. |
| `docs/api.md` | Reference for every public function. |
| `docs/design.md` | Bit numbering, masks, packing, portability, and verification. |

Update `docs/api.md` and the header Doxygen together when an interface changes. Update `docs/design.md` when behaviour changes. Keep it standalone: describe how the library works, not how it was built or which other projects use it. Write prose in British English, one line per paragraph.

## Commits

Use Conventional Commits:

- `feat: ...` new feature
- `fix: ...` bug fix
- `docs: ...` documentation only
- `test: ...` test-only changes
- `chore: ...` build, CI, release work
- `refactor: ...` a change that neither fixes a bug nor adds a feature

Keep the subject under about 70 characters. Use the body to explain why the change is needed.

## Pull requests

- Open an issue first for non-trivial changes so the design can be agreed before implementation.
- Keep pull requests focused: one feature or one fix.
- All CI checks must pass.

## When in doubt

Open an issue. The library is small enough that a modest change to its contract reaches every register image and frame built with it.
