# Integration guide

This guide shows how to add `bitfield` to a project, build it, and run its tests. The API is described in [api.md](api.md), and the bit numbering and packing rules in [design.md](design.md).

## Requirements

- A C11 compiler with `uint16_t` and `uint32_t`. No other dependencies, and no configuration macros.
- For the tests: Meson, Ninja, and Python 3.

## Copy-in

Copy-in is the recommended method for embedded targets. Copy two files into your source tree:

```
include/bitfield.h
src/bitfield.c
```

Compile `bitfield.c` with your other sources and include the header:

```c
#include "bitfield.h"
```

No generated header is needed. `bitfield_version.h` exists only in Meson builds.

## Meson subproject

Put the repository in `subprojects/bitfield`, as a checkout or through a wrap file, then declare the dependency:

```meson
bitfield_dep = dependency('bitfield', fallback: ['bitfield', 'bitfield_dep'])
```

The build calls `meson.override_dependency('bitfield', ...)`, so other subprojects in the same build find `bitfield` by name. Tests are off by default, so a subproject build compiles only the library. Include the header as `"bitfield.h"`.

## Installed package

```sh
meson setup build --buildtype=release --prefix=/usr/local
meson compile -C build
meson install -C build
cc app.c $(pkg-config --cflags --libs bitfield) -o app
```

The headers install to a `bitfield/` subdirectory, and the `pkg-config` flags add that directory to the include path:

<!-- snippet: skip -->
```c
#include "bitfield.h"
#include "bitfield_version.h"
```

## Build and test

```sh
# Library only
meson setup build --buildtype=release
meson compile -C build

# Unit, status, and documentation tests
meson setup build --buildtype=debug -Dbuild_tests=true
meson compile -C build
meson test -C build --verbose
```

Sanitiser, coverage, formatting, and MISRA commands are in [CONTRIBUTING.md](../CONTRIBUTING.md).

## Cross builds

The consuming firmware owns its cross file, linker command file, and memory placement. No cross file ships with this project.

CI compiles the library with warnings as errors using two releases of a vendor C11 compiler for a 16-bit-MAU DSP target, where `CHAR_BIT` is 16, under both of its object formats. This covers the source dialect and type assumptions only. Linking, execution, and the final map-file footprint remain checks for the consumer.
