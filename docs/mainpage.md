# tempoch-cpp API Documentation

`tempoch-cpp` is a **modern, header-only C++17 library** for astronomical
time primitives. It wraps the Rust-based
[`tempoch`](https://github.com/siderust/tempoch) through its C FFI layer
(`tempoch-ffi`), exposing the same two-axis model as Rust: physical
**scale** and external **format** are separate type parameters.

---

## Features

| Feature | Description |
|---------|-------------|
| **`CivilTime`** | Civil UTC date-time struct (year/month/day/hour/min/sec) with nanosecond precision |
| **`Time<scale::S>`** | Canonical instant on a physical timescale, stored as split J2000 seconds |
| **`EncodedTime<S, F>`** | Typed external encoding such as `JulianDate<scale::TT>` or `ModifiedJulianDate<scale::UTC>` |
| **`TimeContext`** | Explicit context for UT1 and historical UTC routes |
| **`Period<T>`** | Inclusive `[start, end]` interval over any supported time representation |
| **Exception hierarchy** | All FFI status codes map to typed C++ exceptions |
| **Header-only** | Drop into any project — no separate compilation step |

---

## Quick Start

```cpp
#include <tempoch/tempoch.hpp>
#include <iostream>

int main() {
    using namespace tempoch;

    CivilTime civil{2026, 7, 15, 22, 0, 0};
    auto utc = Time<scale::UTC>::from_civil(civil);
    auto tt = utc.to<scale::TT>();

    auto jd_tt = tt.to<format::JD>();
    auto jd_utc = utc.to<format::JD>();
    auto mjd_tt = tt.to<format::MJD>();

    std::cout << "Civil UTC : " << civil << "\n";
    std::cout << "JD(TT)    : " << jd_tt << "\n";
    std::cout << "JD(UTC)   : " << jd_utc << "\n";
    std::cout << "MJD(TT)   : " << mjd_tt << "\n";
}
```

---

## Architecture

```
┌──────────────┐
│  C++ user    │   #include <tempoch/tempoch.hpp>
│  code        │
└──────┬───────┘
       │  header-only (inline)
┌──────▼───────┐
│ tempoch-cpp  │   C++17 value types: CivilTime, Time<S>, EncodedTime<S,F>, Period<T>
│  (headers)   │
└──────┬───────┘
       │  extern "C" calls
┌──────▼───────┐
│ tempoch-ffi  │   C ABI (cbindgen-generated from Rust)
│ (.so/.dylib) │
└──────┬───────┘
       │
┌──────▼─────────┐
│ tempoch (Rust) │   Time arithmetic, calendar conversion, period algebra
└────────────────┘
```

---

## Modules

- `tempoch/tempoch.hpp` — umbrella include for the full public API
- `tempoch/time.hpp` — `CivilTime`, `Time<scale::S>`, `JulianDate<scale::S>`, `ModifiedJulianDate<scale::S>`
- `tempoch/period.hpp` — `Period` interval type
- `tempoch/ffi_core.hpp` — FFI helpers and exception hierarchy

## Migration Notes

- Old `JulianDate` means `JulianDate<scale::TT>`.
- Old `MJD` means `ModifiedJulianDate<scale::TT>`.
- Old `TT` means `Time<scale::TT>`.
- Old `UTC` civil construction is now `CivilTime` plus `Time<scale::UTC>::from_civil(...)`.
- Legacy names are available only through opt-in compatibility headers.

---

## Error Model

FFI `tempoch_status_t` codes are translated into typed C++ exceptions:

| Exception | When thrown |
|-----------|-------------|
| `TempochException` | Base class for all tempoch errors |
| `NullPointerError` | FFI returned a null handle |
| `UtcConversionError` | Invalid or out-of-range calendar fields |
| `InvalidPeriodError` | `start > end` or malformed period |
| `NoIntersectionError` | Period intersection on non-overlapping intervals |

---

## Prerequisites

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.15+
- Rust toolchain (`cargo`) — `tempoch-ffi` is built automatically

---

## Building

```bash
git clone --recurse-submodules <url>
cd tempoch-cpp

cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure

./build/01_quickstart
```

---

## Building This Documentation

```bash
cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=ON
cmake --build build --target docs
```

Then open:

- `build/docs/doxygen/html/index.html`
