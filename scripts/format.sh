#!/bin/sh
# SPDX-License-Identifier: MIT
#
# Checks or applies the project's C and C++ formatting with a pinned
# clang-format. uvx fetches that exact release from PyPI and caches it, so the
# result does not depend on a system or editor install. CI runs the check.
#
# Usage: scripts/format.sh          check tracked and new sources
#        scripts/format.sh --fix    rewrite them in place

set -eu

# Raise this to adopt a new release, then run --fix and commit the result.
CLANG_FORMAT_VERSION=23.1.1

usage="usage: scripts/format.sh [--fix]"

if [ "$#" -gt 1 ]; then
  echo "$usage" >&2
  exit 2
fi

case "${1:-}" in
  "") mode=check ;;
  --fix) mode=fix ;;
  -h | --help) echo "$usage"; exit 0 ;;
  *) echo "$usage" >&2; exit 2 ;;
esac

if ! command -v uvx >/dev/null 2>&1; then
  echo "error: uvx not found. Install uv: https://docs.astral.sh/uv/" >&2
  exit 1
fi

cd "$(dirname "$0")/.."

# Tracked sources plus new, unignored ones, so --fix also covers files that
# are not added yet. Paths deleted from the working tree are skipped.
set --
for file in $(git ls-files --cached --others --exclude-standard \
                -- '*.c' '*.h' '*.cpp'); do
  if [ -f "$file" ]; then
    set -- "$@" "$file"
  fi
done

if [ "$#" -eq 0 ]; then
  echo "no C or C++ sources found"
  exit 0
fi

clang_format="clang-format@$CLANG_FORMAT_VERSION"
if [ "$mode" = fix ]; then
  uvx --quiet "$clang_format" -i "$@"
  echo "formatted $# files with $clang_format"
else
  uvx --quiet "$clang_format" --dry-run --Werror "$@"
  echo "$# files match $clang_format"
fi
