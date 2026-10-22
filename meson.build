# SPDX-License-Identifier: MIT

project(
  'bitfield',
  'c',
  version: '1.3.3',
  meson_version: '>=0.57.0',
  default_options: ['c_std=c11', 'warning_level=3'],
  license: 'MIT',
)

# ── Version ────────────────────────────────────────────────────────────────────
# include/bitfield.h defines the version macros, so a copy-in build has them.
# Configuration fails when they differ from the project version.

fs = import('fs')
version_parts = meson.project_version().split('.')
version_macros = {
  'BITFIELD_VERSION_MAJOR': version_parts[0],
  'BITFIELD_VERSION_MINOR': version_parts[1],
  'BITFIELD_VERSION_PATCH': version_parts[2],
  'BITFIELD_VERSION': '"' + meson.project_version() + '"',
}
header_lines = []
foreach line : fs.read('include' / 'bitfield.h').split('\n')
  header_lines += ' '.join(line.split())
endforeach
foreach name, value : version_macros
  if '#define @0@ @1@'.format(name, value) not in header_lines
    error('include/bitfield.h must define @0@ as @1@'.format(name, value))
  endif
endforeach

# ── Include directories ────────────────────────────────────────────────────────

public_headers = include_directories('include')

# ── Application core library ───────────────────────────────────────────────────

library_sources = files('src/bitfield.c')

library = static_library(
  'bitfield',
  library_sources,
  include_directories: public_headers,
  install: true,
)

bitfield_dep = declare_dependency(link_with: library, include_directories: public_headers)

meson.override_dependency('bitfield', bitfield_dep)

install_headers(
  'include/bitfield.h',
  subdir: 'bitfield',
)

# ── pkg-config ─────────────────────────────────────────────────────────────────

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  library,
  name: 'bitfield',
  description: 'Portable bit manipulation library for safety-critical embedded systems',
  subdirs: 'bitfield',
)

# ── Host unit tests ────────────────────────────────────────────────────────────

if get_option('build_tests')
  subdir('tests')
endif
