# tempoch-cpp API Documentation

`tempoch-cpp` is a **modern, header-only C++17 library** for astronomical
time primitives. It wraps the Rust-based
[`tempoch`](https://github.com/siderust/tempoch) through its C FFI layer
(`tempoch-ffi`), exposing `UTC`, `JulianDate`, `MJD`, and `Period` as
idiomatic C++ value types with arithmetic, comparisons, and exception-based
error reporting — no manual FFI plumbing required.

---

## Features

| Feature | Description |
|---------|-------------|
| **`UTC`** | Civil date-time struct (year/month/day/hour/min/sec) with nanosecond precision |
| **`JulianDate`** | Strongly-typed Julian Date with arithmetic and UTC conversion |
| **`MJD`** | Strongly-typed Modified Julian Date; interchangeable with `JulianDate` |
| **`Period`** | Inclusive `[start, end]` MJD interval with intersection, duration, and iteration helpers |
| **Exception hierarchy** | All FFI status codes map to typed C++ exceptions |
| **Header-only** | Drop into any project — no separate compilation step |

---

## Quick Start

```cpp
#include <tempoch/tempoch.hpp>
#include <cstdio>

int main() {
    using namespace tempoch;

    // Construct a UTC civil time
    UTC utc{2026, 7, 15, 22, 0, 0};

    // Convert to Julian Date and MJD
    JulianDate jd  = JulianDate::from_utc(utc);
    MJD        mjd = MJD::from_jd(jd);

    std::printf("JD  = %.6f\n", jd.value());
    std::printf("MJD = %.6f\n", mjd.value());

    // Create a one-day window and check containment
    MJD start = mjd;
    MJD end   = mjd + 1.0;
    Period night{start, end};

    std::printf("Duration: %.2f days\n", night.duration());
    std::printf("Contains midpoint: %s\n",
                night.contains(mjd + 0.5) ? "yes" : "no");

    // Round-trip back to UTC
    UTC back = jd.to_utc();
    std::printf("UTC: %04d-%02d-%02d %02d:%02d:%02d\n",
                back.year, back.month, back.day,
                back.hour, back.minute, back.second);

    return 0;
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
│ tempoch-cpp  │   C++17 value types: UTC, JulianDate, MJD, Period
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
- `tempoch/time.hpp` — `UTC`, `JulianDate`, `MJD` types
- `tempoch/period.hpp` — `Period` interval type
- `tempoch/ffi_core.hpp` — FFI helpers and exception hierarchy

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

./build/time_example
```

---

## Building This Documentation

```bash
cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=ON
cmake --build build --target docs
```

Then open:

- `build/docs/doxygen/html/index.html`
