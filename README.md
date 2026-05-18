# tempoch-cpp

tempoch-cpp is a C++17 library for astronomical time primitives. It provides a header-only C++ API backed by the Rust `tempoch-ffi` engine (via a C FFI), with explicit time scales, explicit encodings, UTC civil conversion, and generic period operations.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-064F8C.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-See%20tempoch-green.svg)](tempoch/LICENSE)

## Features

- Split scale/format model:
  `Time<scale::TT>`, `JulianDate<scale::TT>`, `JulianDate<scale::UTC>`,
  `ModifiedJulianDate<scale::UT1>`, `UnixTime`, and `GpsTime`
- Civil UTC representation with nanosecond precision via `tempoch::CivilTime`
- Explicit conversion flow:
  `CivilTime -> Time<scale::UTC> -> Time<scale::TT> -> to<format::JD>()`
- `TimeContext` for UT1 and historical UTC routes
- Time arithmetic using `qtty-cpp` duration quantities such as `qtty::Day`, `qtty::Hour`, and `qtty::Minute`
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
./build/01_quickstart
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
#include <tempoch/formats/jd.hpp>
#include <tempoch/formats/mjd.hpp>
#include <tempoch/scales/utc.hpp>

int main() {
    using namespace tempoch;

    CivilTime civil{2026, 7, 15, 22, 0, 0};
    auto jd_tt = JulianDate<scale::TT>::from_utc(civil);
    auto mjd_tt = ModifiedJulianDate<scale::TT>::from_jd(jd_tt);
    auto utc = Time<scale::TT>::from_encoded(jd_tt).to<scale::UTC>();
    auto jd_utc = utc.to<format::JD>();

    std::cout << "Civil UTC : " << civil << "\n";
    std::cout << "JD(TT)    : " << jd_tt << "\n";
    std::cout << "JD(UTC)   : " << jd_utc << "\n";
    std::cout << "MJD(TT)   : " << mjd_tt << "\n";

    Period observing_window(mjd_tt, mjd_tt + qtty::Hour(12.0));
    std::cout << "Duration: " << observing_window.duration<qtty::Hour>() << "\n";
}
```

The public headers are now split by concept:

- `tempoch/scales/*.hpp` for physical/civil time-axis tags and scale traits
- `tempoch/formats/*.hpp` for encoding tags and format traits
- `tempoch/tempoch.hpp` as the full umbrella include

### Durations and Arithmetic

Time arithmetic uses `qtty-cpp` quantities. Subtracting two times returns a typed duration, and adding or subtracting a quantity shifts the time value.

```cpp
auto jd0 = tempoch::JulianDate<tempoch::scale::TT>::J2000();
auto jd1 = jd0 + qtty::Hour(12.0);
auto dt = jd1 - jd0;

std::cout << dt.to<qtty::Hour>() << "\n";
```

## API Notes

- Default astronomy-facing code should use TT-based `JulianDate`, `MJD`, and `Period`.
- Civil construction is available directly through `JulianDate<scale::TT>::from_utc(...)` and `ModifiedJulianDate<scale::TT>::from_utc(...)`.
- The explicit typed core remains available through `Time<scale::S>` and `EncodedTime<S, F>` for mixed-scale work.

## Documentation

- `docs/mainpage.md` (API overview)
- `examples/01_quickstart.cpp` (civil UTC -> canonical time -> explicit encodings)
- `examples/02_scales.cpp` (all supported scale conversions)
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

## License

This repository wraps the upstream `tempoch` project (git submodule in `tempoch/`). See `tempoch/LICENSE` for licensing details.
