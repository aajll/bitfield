# AGENTS.md

---

## 1) Project-specific instructions

**Project:** `bitfield`
**Primary goal:** A C11 library of checked bit, range, count and pack operations on 16-bit and 32-bit words, for embedded systems, with no allocation and no global state.

### 1.1 Essential commands

#### Configure and build (library only)

```sh
meson setup build --wipe --buildtype=release -Dbuild_tests=false
meson compile -C build
```

#### Configure, build, and run unit tests

```sh
meson setup build --wipe --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

#### MISRA C:2023 (reads `build/compile_commands.json`)

```sh
misch run --baseline
```

#### Formatting (pinned clang-format through `uvx`)

```sh
scripts/format.sh          # check, as CI does
scripts/format.sh --fix    # rewrite in place
```

### 1.2 Notes

- `meson setup` generates the optional `bitfield_version.h` into the build directory.
- The public header lives at `include/bitfield.h`; do not add a `src/bitfield.h` shim.
- Regenerate the MISRA baseline only with `MISRA_RULE_TEXTS` unset (`env -u MISRA_RULE_TEXTS misch baseline`), because the rule texts change the recorded messages.

---

## 2) Non-negotiable design invariants

- An invalid pointer, position or range writes nothing. `TRUNCATED` is the only status that follows a partial write.
- No shift reaches the width of its operand: range masks are built in a type wider than the word.
- `bitfield16_*` and `bitfield32_*` keep identical semantics.
- Byte buffers carry one octet per `uint_least8_t`; `pack` reads only the low 8 bits and packs least significant byte first.
- Every loop is bounded by a word width or a `uint_least8_t` length. No allocation, recursion, or global state.

---

## 3) CI / source of truth

- CI definitions live in `.github/workflows/ci.yml`.
- Prefer running the same commands locally as CI runs (see §1.1 above).
- Install Python tools (Meson, Ninja, gcovr, misch) with `uv tool install`, never `pip`; CI does the same through `astral-sh/setup-uv`.
- The CI jobs are: tests under ASan/UBSan, a release build, a TI C2000 library build (CGT 22.6.2 and 25.11.1, COFF and EABI, warnings as errors), `clang-format`, a 100 percent line and branch coverage gate, and `misch` against the baseline.

---

## 4) Docs / commit conventions

### Documentation layout

- `README.md` is a short landing page (about 70 lines). Put detail in `docs/`, never in the README; link to the owning file instead.
- `docs/integration.md` owns requirements, install methods, building, and cross builds.
- `docs/api.md` owns the API reference. Change it together with `include/bitfield.h`; keep signatures in fenced `c` blocks and describe them in prose, not tables.
- `docs/design.md` is the single authoritative design record.
- `CONTRIBUTING.md` documents the contributor workflow; `CHANGELOG.md` holds release history; `.github/SECURITY.md` covers vulnerability reporting.
- Code blocks in `README.md`, `CONTRIBUTING.md`, and `docs/` are compiled by the `bitfield docs examples` test (`tests/docs/check_snippets.py`). Each document is compiled as one source file: a `c` fence is file-level code, `<!-- snippet: body -->` wraps statements, and `<!-- snippet: skip -->` excludes pseudo-code. Do not use `skip` to silence a broken example.
- Prose uses British spelling. Do not hard-wrap Markdown: write one line per paragraph. The 80-column limit applies to C source, not Markdown.

### Commits

- Use **Conventional Commits** format when asked to commit.
- Keep commits focused; explain *why* in the message body.

---

## 5) C style expectations

### Build & configuration

- Use the Meson build system. Do not introduce CMake, Make, or other systems.
- Update `meson.build` and `tests/meson.build` when adding or removing files.

### Formatting

- `.clang-format` is present and **mandatory**. Run `scripts/format.sh --fix` before committing. It runs the clang-format release pinned in the script through `uvx`. Do not use a system `clang-format`: another release can format the same code differently.
- Do not reformat unrelated code.
- Key settings: 8-space indent, `BreakBeforeBraces: Linux`, column limit 80.

### Style & correctness

- Match conventions in the existing files (indentation, braces, naming).
- Validate pointer arguments and positions at every public API boundary.
- No heap allocation (`malloc` / `free` / VLAs).
- Use `uint16_t`, `uint32_t`, `uint_least8_t` and `bool` from `<stdint.h>` / `<stdbool.h>`; never plain `int` for fields, and no `uint8_t`.

### Error handling

- Writers return `bitfield_status_t`; readers return `false` or `0` on invalid input.
- No `errno`; no exceptions.

### Comment placement (Doxygen)

- Inline trailing annotations (`/**< ... */`) on `enum`/`struct` members are allowed only when the resulting line fits the 80-column limit.
- If any member's annotation would overrun, move **all** of that aggregate's member docs into a single structured Doxygen block above the type, as an `@details` list of `- ::SYMBOL  description` entries.
- Never mix inline and block forms within one aggregate, and never leave a trailing comment that clang-format would wrap onto a second line.
- After editing, run `scripts/format.sh` and an 80-column scan.

### Testing

- Run `meson test -C build` after every change.
- Add a test case for each bug fix, in both families.
- Tests live in `tests/test_bitfield.c` and `tests/test_status.c`; both must pass.
- Keep `src/` and `include/` at the coverage gate (100 percent line and branch).
