#! /usr/bin/env bash

set -euo pipefail

readonly ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODE=""
SHOW_DIFF=1
CLANG_FORMAT_BIN="${CLANG_FORMAT_BIN:-}"

usage() {
  cat <<'EOF'
Usage: ./clang_format.sh [--check | --fix] [--no-diff] [--help]

Options:
  --check    Check formatting and fail if files need changes.
  --fix      Apply formatting changes in place.
  --no-diff  In check mode, do not print unified diffs.
  --help     Show this help message.

Environment:
  CLANG_FORMAT_BIN  Path or command name to clang-format binary.
EOF
}

parse_args() {
  if [[ $# -eq 0 ]]; then
    MODE="check"
    return
  fi

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --check)
        MODE="check"
        ;;
      --fix)
        MODE="fix"
        ;;
      --no-diff)
        SHOW_DIFF=0
        ;;
      --help|-h)
        usage
        exit 0
        ;;
      *)
        echo "Unknown argument: $1" >&2
        usage >&2
        exit 2
        ;;
    esac
    shift
  done

  if [[ -z "$MODE" ]]; then
    echo "Select one mode: --check or --fix" >&2
    usage >&2
    exit 2
  fi
}

resolve_clang_format() {
  if [[ -n "$CLANG_FORMAT_BIN" ]] && command -v "$CLANG_FORMAT_BIN" >/dev/null 2>&1; then
    return
  fi

  for candidate in clang-format-18 clang-format clang-format-17 clang-format-16 clang-format-15 clang-format-14; do
    if command -v "$candidate" >/dev/null 2>&1; then
      CLANG_FORMAT_BIN="$candidate"
      return
    fi
  done

  echo "clang-format binary not found. Install clang-format or set CLANG_FORMAT_BIN." >&2
  exit 1
}

collect_files() {
  local file
  local -a collected=()

  mapfile -t collected < <(git -C "$ROOT_DIR" ls-files '*.hpp' '*.cpp' | sort)

  FILES=()
  for file in "${collected[@]}"; do
    FILES+=("$ROOT_DIR/$file")
  done
}

check_files() {
  echo "Checking formatting with $CLANG_FORMAT_BIN"
  "$CLANG_FORMAT_BIN" --dry-run --Werror "${FILES[@]}"
  echo "clang-format check passed."
}

fix_files() {
  local file

  echo "Applying formatting with $CLANG_FORMAT_BIN"

  for file in "${FILES[@]}"; do
    "$CLANG_FORMAT_BIN" -i "$file"
  done

  echo "Formatting applied to ${#FILES[@]} files."
}

main() {
  parse_args "$@"
  resolve_clang_format

  cd "$ROOT_DIR"
  collect_files

  if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "No tracked .hpp or .cpp files found."
    exit 0
  fi

  case "$MODE" in
    check) check_files ;;
    fix) fix_files ;;
  esac
}

main "$@"
