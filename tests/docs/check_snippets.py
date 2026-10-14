#!/usr/bin/env python3
"""Compile or syntax-check the code examples in a project's markdown.

usage: check_snippets.py --lang LANG [--root DIR] [--doc GLOB ...]
                         [--prelude LINE ...] [--keep DIR] [--timeout S]
                         [-- COMMAND ...]

Copy this file into the repository, for example as tests/docs/check_snippets.py,
and register it with the project's test runner so CI runs it. It needs only
the Python standard library.

For each markdown file matched by --doc (default: README.md, CONTRIBUTING.md,
and docs/**/*.md), the blocks fenced as LANG form one generated source file in
document order, so a later block can use an earlier declaration. The --prelude
lines come first. COMMAND runs once per file, with {source} and {object}
replaced by generated paths. Without a COMMAND, --keep is required and the
files are only written for inspection.

A block is file-level code unless the nearest non-blank line above its opening
fence is a marker comment, which markdown renderers hide:

  <!-- snippet: body -->   statements, wrapped in a generated function
  <!-- snippet: skip -->   pseudo-code, or a copy of a real definition

Diagnostics name the markdown file and line. C and C++ sources carry #line
directives, and other tool output that names a generated file is rewritten.

example (C):
  check_snippets.py --lang c --prelude '#include "mylib.h"' -- \\
      cc -std=c11 -Wall -Wextra -Werror -Iinclude -c {source} -o {object}

Exit status: 0 when every file passes, 1 when a file fails, a marker is
misplaced, or no block matched, and 2 on a usage error.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import final

DEFAULT_DOCS = ("README.md", "CONTRIBUTING.md", "docs/**/*.md")
FENCE = re.compile(r"^(\s*)(`{3,}|~{3,})\s*([^\s`{]*)")
MARKER = re.compile(r"^\s*<!--\s*snippet:\s*(\S*)\s*-->\s*$")
KINDS = ("body", "skip")


@final
@dataclass(frozen=True)
class Language:
    fences: frozenset[str]
    extension: str
    line_directive: bool
    body_open: tuple[str, ...]
    body_close: tuple[str, ...]
    body_indent: str = ""


LANGUAGES = {
    "c": Language(
        frozenset({"c"}),
        "c",
        True,
        ("void snippet_body_{index}(void);", "void snippet_body_{index}(void)", "{"),
        ("}",),
    ),
    "cpp": Language(
        frozenset({"cpp", "c++", "cxx"}),
        "cpp",
        True,
        ("void snippet_body_{index}();", "void snippet_body_{index}()", "{"),
        ("}",),
    ),
    "python": Language(
        frozenset({"python", "py", "python3"}),
        "py",
        False,
        ("def _snippet_body_{index}() -> None:",),
        ("    pass",),
        "    ",
    ),
    "sh": Language(
        frozenset({"sh", "bash", "shell"}),
        "sh",
        False,
        ("_snippet_body_{index}() {",),
        (":", "}"),
    ),
}


class SnippetError(Exception):
    pass


@final
@dataclass
class Block:
    line: int
    kind: str
    code: list[str] = field(default_factory=list)


@final
class Arguments(argparse.Namespace):
    def __init__(self) -> None:
        super().__init__()
        self.lang: str = ""
        self.root: Path = Path(".")
        self.docs: list[str] = []
        self.prelude: list[str] = []
        self.keep: Path | None = None
        self.timeout: float = 120.0


def parse(path: Path, name: str, fences: frozenset[str]) -> list[Block]:
    """Return the blocks fenced as one of fences, each with its marker kind."""
    blocks: list[Block] = []
    fence = ""
    indent = 0
    current: Block | None = None
    marker = ""
    marker_line = 0
    text = path.read_text(encoding="utf-8")
    for number, line in enumerate(text.splitlines(), start=1):
        stripped = line.strip()
        if fence:
            if stripped.startswith(fence) and stripped == fence[0] * len(stripped):
                if current is not None:
                    blocks.append(current)
                fence = ""
                current = None
            elif current is not None:
                prefix = line[:indent]
                current.code.append(line[indent:] if not prefix.strip() else line)
            continue
        if not stripped:
            continue
        opening = FENCE.match(line)
        if opening:
            indent = len(opening.group(1))
            fence = opening.group(2)
            if opening.group(3).lower() in fences:
                current = Block(number + 1, marker or "file")
            marker = ""
            continue
        if marker:
            raise SnippetError(f"{name}:{marker_line}: marker is not above a block")
        match = MARKER.match(line)
        if match:
            if match.group(1) not in KINDS:
                raise SnippetError(
                    f"{name}:{number}: unknown marker {match.group(1)!r}"
                )
            marker = match.group(1)
            marker_line = number
    if marker:
        raise SnippetError(f"{name}:{marker_line}: marker is not above a block")
    if fence:
        raise SnippetError(f"{name}: unterminated code block")
    return blocks


def assemble(
    name: str, language: Language, prelude: list[str], blocks: list[Block]
) -> tuple[str, list[int | None]]:
    """Return the generated source and the markdown line of each source line."""
    lines: list[str] = []
    origin: list[int | None] = []
    quoted = name.replace("\\", "\\\\").replace('"', '\\"')

    def emit(text: str, line: int | None = None) -> None:
        lines.append(text)
        origin.append(line)

    for text in prelude:
        emit(text)
    for index, block in enumerate(blocks, start=1):
        if block.kind == "skip":
            continue
        body = block.kind == "body"
        if body:
            for text in language.body_open:
                emit(text.replace("{index}", str(index)))
        if language.line_directive:
            emit(f'#line {block.line} "{quoted}"')
        prefix = language.body_indent if body else ""
        for offset, text in enumerate(block.code):
            emit(prefix + text if text.strip() else text, block.line + offset)
        if body:
            for text in language.body_close:
                emit(text)
    return "\n".join(lines) + "\n", origin


def relabel(output: str, source: Path, name: str, origin: list[int | None]) -> str:
    """Point tool output that names the generated file at the markdown line."""
    pattern = re.compile(re.escape(str(source)) + r'(", line |: line |:)(\d+)')

    def replace(match: re.Match[str]) -> str:
        number = int(match.group(2))
        line = origin[number - 1] if 0 < number <= len(origin) else None
        if line is None:
            return match.group(0)
        return f"{name}{match.group(1)}{line}"

    return pattern.sub(replace, output)


def check(args: Arguments, command: list[str], work: Path) -> int:
    root = args.root.resolve()
    language = LANGUAGES[args.lang]
    patterns = args.docs or list(DEFAULT_DOCS)
    documents = sorted(
        {path for pattern in patterns for path in root.glob(pattern) if path.is_file()}
    )
    passed = 0
    skipped = 0
    failures: list[str] = []

    for index, path in enumerate(documents, start=1):
        name = path.relative_to(root).as_posix()
        try:
            blocks = parse(path, name, language.fences)
        except SnippetError as error:
            failures.append(str(error))
            continue
        active = sum(1 for block in blocks if block.kind != "skip")
        skipped += len(blocks) - active
        if not active:
            continue
        text, origin = assemble(name, language, args.prelude, blocks)
        stem = name.removesuffix(".md").replace("/", "_")
        source = work / f"{index:02d}-{stem}.{language.extension}"
        _ = source.write_text(text, encoding="utf-8")
        if not command:
            passed += active
            print(f"{name}: {active} blocks written to {source}")
            continue
        target = source.with_suffix(".o")
        argv = [
            arg.replace("{source}", str(source)).replace("{object}", str(target))
            for arg in command
        ]
        try:
            result = subprocess.run(
                argv,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=args.timeout,
                check=False,
            )
        except (OSError, subprocess.TimeoutExpired) as error:
            failures.append(f"{name}: {error}")
            continue
        if result.returncode != 0:
            output = relabel(result.stdout, source, name, origin)
            failures.append(f"{name}: examples fail\n{output}")
            continue
        passed += active
        print(f"{name}: {active} blocks pass")

    for failure in failures:
        print(failure, file=sys.stderr)
    verb = "checked" if command else "written"
    print(f"{args.lang}: {passed} blocks {verb}, {skipped} skipped")
    if passed == 0 and not failures:
        print(f"no {args.lang} blocks found under {root}", file=sys.stderr)
        return 1
    return 1 if failures else 0


def main(argv: list[str]) -> int:
    options, command = argv, []
    if "--" in argv:
        split = argv.index("--")
        options, command = argv[:split], argv[split + 1 :]

    parser = argparse.ArgumentParser(
        prog="check_snippets.py",
        usage=argparse.SUPPRESS,
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    _ = parser.add_argument("--lang", required=True, choices=sorted(LANGUAGES))
    _ = parser.add_argument("--root", type=Path)
    _ = parser.add_argument("--doc", dest="docs", action="append", metavar="GLOB")
    _ = parser.add_argument("--prelude", action="append", metavar="LINE")
    _ = parser.add_argument("--keep", type=Path, metavar="DIR")
    _ = parser.add_argument("--timeout", type=float, metavar="S")
    args = parser.parse_args(options, namespace=Arguments())

    if not args.root.is_dir():
        parser.error(f"not a directory: {args.root}")
    if command and not any("{source}" in arg for arg in command):
        parser.error("COMMAND must contain {source}")
    if not command and args.keep is None:
        parser.error("give a COMMAND after --, or --keep DIR")

    with tempfile.TemporaryDirectory(prefix="snippets-") as temporary:
        work = Path(temporary)
        if args.keep is not None:
            work = args.keep.resolve()
            work.mkdir(parents=True, exist_ok=True)
        return check(args, command, work)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
