# tempoch-cpp

tempoch-cpp is a C++17 library for astronomical time primitives. It provides a header-only C++ API backed by the Rust `tempoch-ffi` engine (via a C FFI), with typed time scales, UTC conversions, and period operations.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-064F8C.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-See%20tempoch-green.svg)](tempoch/LICENSE)

## Features

- Strongly typed time values such as `JulianDate`, `MJD`, `UTC`, `TT`, `TAI`, `TDB`, `TCG`, `TCB`, `GPS`, `UT`, `JDE`, and `UnixTime`
- Civil UTC representation with nanosecond precision via `tempoch::CivilTime`
- Cross-scale conversion helpers such as `from_utc()`, `to_utc()`, `to_jd()`, and `to_mjd()`
- Time arithmetic using `qtty-cpp` duration quantities such as `qtty::Day`, `qtty::Hour`, and `qtty::Minute`
- `UT::delta_t()` with checked UT1 horizon errors from the Rust FFI
- `Period<T>` intervals with typed start/end access, intersection, and duration conversion
- CMake target (`tempoch_cpp`) for straightforward integration

The C++ wrapper intentionally mirrors the current `tempoch-ffi` surface. Rust-only features such as `serde`, tagged wrappers, and runtime-bundle refresh/context management are not exposed in `tempoch-cpp` yet.

## Requirements

- CMake 3.15+
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- Rust/Cargo (builds `tempoch-ffi`)

## Build and Test

```bash
git clone --recurse-submodules <repo-url>
cd tempoch-cpp

cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/time_example
```

If you cloned without submodules:

```bash
git submodule update --init --recursive
```

## Usage

```cpp
#include <iostream>
#include <qtty/qtty.hpp>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

int main() {
    UTC utc(2026, 7, 15, 22, 0, 0);

    auto jd = JulianDate::from_utc(utc);
    auto mjd = MJD::from_jd(jd);

    std::cout << "UTC: " << utc << "\n";
    std::cout << "JD:  " << jd << "\n";
    std::cout << "MJD: " << mjd << "\n";

    Period observing_window(MJD(60200.0), MJD(60200.5));
    std::cout << "Duration: "
              << observing_window.duration<qtty::Hour>()
              << "\n";
}
```

### Durations and Arithmetic

Time arithmetic uses `qtty-cpp` quantities. Subtracting two times returns a typed duration, and adding or subtracting a quantity shifts the time value.

```cpp
auto jd0 = JulianDate::J2000();
auto jd1 = jd0 + qtty::Hour(12.0);
auto dt = jd1 - jd0;

std::cout << dt.to<qtty::Hour>() << "\n";
```

## Documentation

- `docs/mainpage.md` (API overview)
- `examples/time_example.cpp` (basic usage)
- `examples/timescales.cpp` (all supported scale conversions plus `ΔT`)
- `include/tempoch/tempoch.hpp` (umbrella public header)

## Integration

### Add as a Subdirectory

```cmake
add_subdirectory(path/to/tempoch-cpp)
target_link_libraries(your_target PRIVATE tempoch_cpp)
```

### Install and Use `find_package`

```bash
cmake --install build
```

```cmake
find_package(tempoch_cpp REQUIRED)
target_link_libraries(your_target PRIVATE tempoch::tempoch_cpp)
```

## Deployment

Packages for Debian/Ubuntu (`.deb`) and RHEL/Fedora/openSUSE (`.rpm`) are built
with CPack.

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt-get install cmake ninja-build rpm

# RHEL/Fedora
sudo dnf install cmake ninja-build dpkg
```

### Build the packages

```bash
git clone --recurse-submodules <repo-url>
cd tempoch-cpp

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DTEMPOCH_BUILD_DOCS=OFF
cmake --build build --parallel

# Install headers + cmake config to a staging prefix
cmake --install build --prefix build/staging

# Generate .deb and .rpm in build/packages/
cd build
cpack --config CPackConfig.cmake -G "DEB;RPM" -B packages
ls packages/
```

### Install on the target system

```bash
# Debian/Ubuntu
sudo dpkg -i packages/tempoch-cpp-*.deb

# RHEL/Fedora/openSUSE
sudo rpm -i packages/tempoch-cpp-*.rpm
```

After installation, headers land in `/usr/local/include/tempoch/` and the
shared library in `/usr/local/lib/`.  CMake consumers can then use:

```cmake
find_package(tempoch_cpp REQUIRED)
target_link_libraries(your_target PRIVATE tempoch::tempoch_cpp)
```

> **Note:** Pre-built `.deb` and `.rpm` packages are also automatically attached
> to every [GitHub Release](https://github.com/Siderust/tempoch-cpp/releases).

## License

This repository wraps the upstream `tempoch` project (git submodule in `tempoch/`). See `tempoch/LICENSE` for licensing details.
