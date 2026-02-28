#!/usr/bin/env bash
set -euo pipefail

# Local CI runner that mirrors .github/workflows/ci.yml

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

bold() { printf '\e[1m%s\e[0m\n' "$*"; }
error() { printf '\e[31m%s\e[0m\n' "$*" >&2; }

run_lint() {
    bold "==> Lint: configure + clang-format + clang-tidy"
    rm -rf build
    cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    mapfile -t format_files < <(git ls-files '*.hpp' '*.cpp')
    if [ ${#format_files[@]} -gt 0 ]; then
        bold "Running clang-format..."
        clang-format --dry-run --Werror "${format_files[@]}"
    else
        echo "No C++ files to format"
    fi

    mapfile -t tidy_files < <(git ls-files '*.cpp')
    if [ ${#tidy_files[@]} -gt 0 ]; then
        bold "Running clang-tidy..."
        for f in "${tidy_files[@]}"; do
            echo "clang-tidy: ${f}"
            clang-tidy -p build --warnings-as-errors='*' "${f}"
        done
    else
        echo "No C++ source files for clang-tidy"
    fi
}

run_build_test_docs() {
    bold "==> Build + Test + Docs"
    rm -rf build
    cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=ON
    export CMAKE_BUILD_PARALLEL_LEVEL=${CMAKE_BUILD_PARALLEL_LEVEL:-2}
    cmake --build build --target test_tempoch
    ctest --test-dir build --output-on-failure -L tempoch_cpp
    cmake --build build --target docs
}

run_coverage() {
    bold "==> Coverage"
    rm -rf build-coverage coverage.xml coverage_html code-coverage-results.md
    cmake -S . -B build-coverage -G Ninja \
        -DTEMPOCH_BUILD_DOCS=OFF \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="--coverage" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage"

    cmake --build build-coverage --target test_tempoch
    ctest --test-dir build-coverage --output-on-failure -L tempoch_cpp

    mkdir -p coverage_html
    gcovr \
        --root . \
        --exclude 'build-coverage/.*' \
        --exclude 'tempoch/.*' \
        --exclude 'tests/.*' \
        --exclude 'examples/.*' \
        --xml \
        --output coverage.xml

    gcovr \
        --root . \
        --exclude 'build-coverage/.*' \
        --exclude 'tempoch/.*' \
        --exclude 'tests/.*' \
        --exclude 'examples/.*' \
        --html-details \
        --output coverage_html/index.html
}

cd "${ROOT_DIR}"

# Check required tools
bold "Checking required tools..."
for tool in cmake ninja clang-format clang-tidy cargo gcovr; do
    if ! command -v "$tool" &> /dev/null; then
        error "ERROR: $tool not found. Please install it."
        exit 1
    fi
done

bold "Using Rust toolchain:"
rustup show active-toolchain

# Check submodules
bold "Checking submodules..."
if [ ! -f tempoch/Cargo.toml ]; then
    error "ERROR: tempoch/Cargo.toml not found. Initialize submodules with: git submodule update --init --recursive"
    exit 1
fi
if [ ! -f tempoch/tempoch-ffi/Cargo.toml ]; then
    error "ERROR: tempoch/tempoch-ffi/Cargo.toml not found. Initialize submodules with: git submodule update --init --recursive"
    exit 1
fi

bold "Submodule revisions:"
echo "tempoch:     $(git -C tempoch rev-parse HEAD) ($(git -C tempoch describe --tags --always 2>/dev/null || true))"
echo "tempoch-ffi: $(git -C tempoch/tempoch-ffi rev-parse HEAD) ($(git -C tempoch/tempoch-ffi describe --tags --always 2>/dev/null || true))"

# Run CI steps
run_lint
run_build_test_docs
run_coverage

bold "==> All CI steps completed successfully!"
