#!/usr/bin/env python3

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path


VERSION_RE = re.compile(r"GLCP_GL_VERSION_\d+_\d+")
HEADER_EXTERN_RE = re.compile(r"^\s*extern\s+(PFNGL[A-Za-z0-9_]+PROC)\s+(gl[A-Za-z0-9_]+)\s*;")
SOURCE_DEFINE_RE = re.compile(r"^\s*(PFNGL[A-Za-z0-9_]+PROC)\s+(gl[A-Za-z0-9_]+)\s*=\s*NULL;")
SOURCE_INIT_RE = re.compile(
    r'^\s*(gl[A-Za-z0-9_]+)\s*=\s*\((PFNGL[A-Za-z0-9_]+PROC)\)glcpGetProcAddress\("(gl[A-Za-z0-9_]+)"\);'
)
SOURCE_FINALIZE_RE = re.compile(r"^\s*(gl[A-Za-z0-9_]+)\s*=\s*NULL;")
INVENTORY_RE = re.compile(r"GLCP_TEST_FUNCTION\((gl[A-Za-z0-9_]+)\)")


def current_versions_for_line(line: str):
    if not line.startswith("#if"):
        return []
    return VERSION_RE.findall(line)


def parse_blocked_entries(path: Path, line_re: re.Pattern[str]):
    stack = []
    by_version = defaultdict(dict)

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if line.startswith("#if"):
            stack.append(current_versions_for_line(line))
            continue
        if line.startswith("#endif"):
            if stack:
                stack.pop()
            continue

        match = line_re.match(raw_line)
        if not match:
            continue

        active_versions = [version for versions in stack for version in versions]
        for version in active_versions:
            by_version[version][match.group(2) if match.lastindex and match.lastindex >= 2 else match.group(1)] = match.groups()

    return by_version


def parse_source_sections(path: Path):
    stack = []
    mode = "definitions"
    definitions = defaultdict(dict)
    initializes = defaultdict(dict)
    finalizes = defaultdict(set)

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()

        if line.startswith("void glcp") and line.endswith("Initialize()"):
            mode = "initialize"
            continue
        if line.startswith("void glcp") and line.endswith("Finalize()"):
            mode = "finalize"
            continue

        if line.startswith("#if"):
            stack.append(current_versions_for_line(line))
            continue
        if line.startswith("#endif"):
            if stack:
                stack.pop()
            continue

        active_versions = [version for versions in stack for version in versions]
        if not active_versions:
            continue

        if mode == "definitions":
            match = SOURCE_DEFINE_RE.match(raw_line)
            if match:
                prototype, function = match.group(1), match.group(2)
                for version in active_versions:
                    definitions[version][function] = prototype
            continue

        if mode == "initialize":
            match = SOURCE_INIT_RE.match(raw_line)
            if match:
                function, prototype, proc_name = match.groups()
                if function != proc_name:
                    raise AssertionError(
                        f"{path}: initialization name mismatch for {function} vs {proc_name}"
                    )
                for version in active_versions:
                    initializes[version][function] = prototype
            continue

        if mode == "finalize":
            match = SOURCE_FINALIZE_RE.match(raw_line)
            if match:
                function = match.group(1)
                for version in active_versions:
                    finalizes[version].add(function)

    return definitions, initializes, finalizes


def parse_inventory(path: Path):
    stack = []
    by_version = defaultdict(set)

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if line.startswith("#if"):
            stack.append(current_versions_for_line(line))
            continue
        if line.startswith("#endif"):
            if stack:
                stack.pop()
            continue

        match = INVENTORY_RE.search(raw_line)
        if not match:
            continue

        function = match.group(1)
        active_versions = [version for versions in stack for version in versions]
        for version in active_versions:
            by_version[version].add(function)

    return by_version


def assert_equal_sets(label: str, version: str, left_name: str, left, right_name: str, right):
    if left != right:
        only_left = sorted(left - right)
        only_right = sorted(right - left)
        raise AssertionError(
            f"{label} {version}: {left_name} != {right_name}; "
            f"only in {left_name}: {only_left}; only in {right_name}: {only_right}"
        )

def parse_versioned_inventory(path: Path):
    stack = []
    by_version = defaultdict(set)

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if line.startswith("#if"):
            stack.append(current_versions_for_line(line))
            continue
        if line.startswith("#endif"):
            if stack:
                stack.pop()
            continue

        match = re.search(r"GLCP_TEST_VERSION_FUNCTION\((GLCP_GL_VERSION_\d+_\d+),\s*(gl[A-Za-z0-9_]+)\)", raw_line)
        if not match:
            continue

        version, function = match.groups()
        active_versions = [version_name for versions in stack for version_name in versions]
        if active_versions:
            for active_version in active_versions:
                by_version[active_version].add(function)
        else:
            by_version[version].add(function)

    return by_version


def validate_loader(label: str, header: Path, source: Path, inventory: Path, versioned_inventory: Path | None):
    header_entries = parse_blocked_entries(header, HEADER_EXTERN_RE)
    source_definitions, source_initializes, source_finalizes = parse_source_sections(source)
    inventory_entries = parse_inventory(inventory)
    versioned_inventory_entries = parse_versioned_inventory(versioned_inventory) if versioned_inventory else {}

    versions = sorted(
        set(header_entries)
        | set(source_definitions)
        | set(source_initializes)
        | set(source_finalizes)
        | set(inventory_entries)
        | set(versioned_inventory_entries)
    )

    if not versions:
        raise AssertionError(f"{label}: no versioned entries were found")

    print(f"{label}: validating {len(versions)} version blocks")

    total_functions = 0
    for version in versions:
        header_functions = set(header_entries.get(version, {}))
        definition_functions = set(source_definitions.get(version, {}))
        initialize_functions = set(source_initializes.get(version, {}))
        finalize_functions = set(source_finalizes.get(version, set()))
        inventory_functions = set(inventory_entries.get(version, set()))
        versioned_inventory_functions = set(versioned_inventory_entries.get(version, set()))

        assert_equal_sets(label, version, "header", header_functions, "source definitions", definition_functions)
        assert_equal_sets(label, version, "header", header_functions, "source initializers", initialize_functions)
        assert_equal_sets(label, version, "header", header_functions, "source finalizers", finalize_functions)
        assert_equal_sets(label, version, "inventory", inventory_functions, "source initializers", initialize_functions)
        if versioned_inventory:
            assert_equal_sets(label, version, "versioned inventory", versioned_inventory_functions, "source initializers", initialize_functions)

        for function, header_groups in header_entries.get(version, {}).items():
            header_proto = header_groups[0]
            if source_definitions[version].get(function) != header_proto:
                raise AssertionError(f"{label} {version}: prototype mismatch for {function} in definitions")
            if source_initializes[version].get(function) != header_proto:
                raise AssertionError(f"{label} {version}: prototype mismatch for {function} in initializers")

        count = len(header_functions)
        total_functions += count
        print(f"{label}: {version} -> {count} functions")

    print(f"{label}: validated {total_functions} functions across {len(versions)} version blocks")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--label", required=True)
    parser.add_argument("--header", required=True)
    parser.add_argument("--source", required=True)
    parser.add_argument("--inventory", required=True)
    parser.add_argument("--versioned-inventory")
    args = parser.parse_args()

    try:
        validate_loader(
            args.label,
            Path(args.header),
            Path(args.source),
            Path(args.inventory),
            Path(args.versioned_inventory) if args.versioned_inventory else None,
        )
    except AssertionError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
