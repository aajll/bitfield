foreach t : ['bitfield', 'status']
  exe = executable(
    'test_' + t,
    'test_' + t + '.c',
    dependencies: [bitfield_dep],
    install: false,
  )
  test(t + ' tests', exe)
endforeach

# Compiles every C block in README.md, CONTRIBUTING.md and docs/ against the
# public header, so a documented signature cannot drift from the code.
test(
  'bitfield docs examples',
  import('python').find_installation(),
  args: [
    files('docs/check_snippets.py'),
    '--root', meson.project_source_root(),
    '--lang', 'c',
    '--prelude', '#include "bitfield.h"',
    '--',
  ] + meson.get_compiler('c').cmd_array() + [
    '-std=c11',
    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Werror',
    '-Wno-unused-function',
    '-Wno-unused-variable',
    '-I' + (meson.project_source_root() / 'include'),
    '-c', '{source}', '-o', '{object}',
  ],
  timeout: 60,
)
