# tempoch-cpp

Header-only C++ wrapper over `tempoch-ffi` for astronomical time primitives.

## Modules

- **tempoch::UTC**: civil date-time struct with nanosecond precision.
- **tempoch::JulianDate**: strongly-typed Julian Date value wrapper.
- **tempoch::MJD**: strongly-typed Modified Julian Date value wrapper.
- **tempoch::Period**: inclusive `[start, end]` interval in MJD.

## Error model

The wrapper translates C `tempoch_status_t` error codes into typed C++
exceptions:

- `TempochException`
- `NullPointerError`
- `UtcConversionError`
- `InvalidPeriodError`
- `NoIntersectionError`

## Build docs

Generate API docs with:

```bash
cmake -S . -B build -G Ninja -DTEMPOCH_BUILD_DOCS=ON
cmake --build build --target docs
```

Generated HTML is located in:

`build/docs/doxygen/html/index.html`
