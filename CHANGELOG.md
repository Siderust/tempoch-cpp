# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.5.1] - 2026-06-01

### Changed

- Refreshed the vendored `qtty-cpp` submodule to the latest patch release,
  including the new angular trig helpers and the additional `Ratio` FFI unit
  support.
- Expanded the C++ time adapter with the new GNSS scale and week helpers
  introduced on the Rust side.

## [0.5.0] - 2026-05-18

### Breaking

- Removed `EncodedTime<S, F>::to_time()` / `to_time_with()` from the public C++ API; decode scalar
  transport values into canonical storage using `Time<S>::from_encoded(encoded)` /
  `Time<S>::from_encoded_with(encoded, ctx)` (parity with Rust `Time::to_j2000s()` after dropping the
  historical aliases).

## [0.4.0] - 2026-05-15

### Added

- New FFI exception mappings in `include/tempoch/ffi_core.hpp` for generic conversion failures and invalid format identifiers via `ConversionFailedError` and `InvalidFormatIdError`.

### Changed

- Refactored the public time model around explicit physical scales and external encodings: `Time<scale::S>` represents canonical instants, while `EncodedTime<S, F>` covers representations such as `JulianDate<scale::TT>`, `ModifiedJulianDate<scale::UTC>`, `UnixTime`, and `GpsTime`.
- `include/tempoch/time.hpp`, `time_base.hpp`, `scales.hpp`, and `period.hpp` now expose the split scale/format API, including `TimeContext` for UT1 and historical UTC conversions.
- `JulianDate::from_utc(...)`, `MJD::from_utc(...)`, `to_utc()`, `to_mjd()`, `to_jd()`, and direct `+=` / `-=` arithmetic are part of the supported ergonomic surface on top of the typed core.
- `Period<T>` remains generalized across supported time representations through `TimeTraits`.
- README, Doxygen docs, examples, and tests were updated to show the ergonomic TT-default workflow first, with the explicit typed API kept available for advanced mixed-scale use.
- Updated the vendored `tempoch` and `qtty-cpp` submodules to the snapshots used by this branch.

## [0.3.1] - 2026-05-11

### Fixed

- `CMakeLists.txt`: guarded all `install()` rules and `CMakePackageConfigHelpers`
  calls inside `if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)` so
  that tempoch-cpp artifacts are no longer included in parent packages
  (siderust-cpp) when tempoch-cpp is used as a CMake subdirectory.  This
  eliminates file-conflict errors such as
  `trying to overwrite '/usr/include/tempoch/ffi_core.hpp'` when both packages
  are installed simultaneously.
- GitHub Actions `build-test-docs` and `coverage` now run the repository
  helper scripts and build the correct `test_tempoch` target instead of the
  non-existent `test_ffi` target.
- `scripts/fmt.sh` now delegates to `scripts/lint.sh` instead of referencing a
  missing `./clang_format.sh` helper, so local `scripts/ci.sh fmt` / `all`
  runs use the same clang-format and clang-tidy pipeline as CI.
- `docs/Doxyfile.in` no longer points `PROJECT_LOGO` at a missing
  `../../public/logo.png`, eliminating a Doxygen configuration error from the
  docs build.
- Documentation targets are now project-specific: `tempoch-cpp` disables nested
  `qtty-cpp` docs when vendored and builds its own `tempoch_docs` target, with
  `docs` kept only as the top-level alias. This prevents nested subprojects
  from hijacking `cmake --build ... --target docs`.

### Changed

- CPack DEB dependency on `qtty-cpp` now carries a minimum version constraint:
  `qtty-cpp (>= 0.4.2)`.
- CPack RPM dependency updated to `qtty-cpp >= 0.4.2`.

## [0.3.0] - 2026-05-09

### Added

- CI workflows and local CI scripts: linting (clang-format, clang-tidy), build, test, coverage, packaging, and release automation.
- Repository scripts: `scripts/fmt.sh`, `scripts/lint.sh`, `scripts/build.sh`, `scripts/test.sh`, `scripts/docs.sh`, `scripts/coverage.sh`, `scripts/ci.sh`, and `scripts/lib.sh` to standardize developer and CI tasks.
- GitHub Actions workflow for C++ linting: `.github/workflows/ci-lint.yml` (clang-format + clang-tidy checks).

### Changed

- Refactor: tempoch time-scale internals were refactored to simplify scale dispatch and improve maintainability; related CI scripts and CMake flags were updated accordingly.
- Chore: removed example artifacts `time_example` and `timescales` from the repository packaging.

## [0.2.1] - 2026-05-04

### Changed

- Updated the vendored Rust dependency line to the `tempoch 0.4.3` release.
- Switched scale-id dispatch to the named `tempoch_ffi.h` constants instead of hard-coded integers.
- `UT::delta_t()` now uses the checked FFI path and throws `Ut1HorizonExceededError` when the requested epoch lies beyond the compiled UT1 horizon.
- Expanded the examples/README to demonstrate the full C++ wrapper surface and explicitly document the remaining Rust-only features.

## [0.2.0] - 2026-03-08

### Added

- `CivilTime` struct for UTC date-time breakdown, replacing forward declaration in scales.
  - Supports construction from individual components and conversion to/from C FFI `tempoch_utc_t`.
  - Stream operator support for formatted output as YYYY-MM-DD HH:MM:SS[.nnnnnnnnn].
- `TimeScaleTraits` specializations for multiple time scales:
  - `JDScale` — Julian Date (days since −4712‑01‑01T12:00 TT)
  - `MJDScale` — Modified Julian Date (JD − 2 400 000.5)
  - `UTCScale` — UTC (internally represented as MJD days)
  - `TTScale` — Terrestrial Time
  - `TAIScale` — International Atomic Time
  - `TDBScale` — Barycentric Dynamical Time
  - `TCGScale` — Geocentric Coordinate Time
  - `TCBScale` — Barycentric Coordinate Time
  - `GPSScale` — GPS Time
  - `UTScale` — Universal Time (UT1)
  - `JDEScale` — Julian Ephemeris Date
  - `UnixTimeScale` — Unix (POSIX) time
- Type aliases for convenience:
  - `using JulianDate = Time<JDScale>`
  - `using MJD = Time<MJDScale>`
  - `using TDB = Time<TDBScale>`
  - `using TT = Time<TTScale>`
  - `using TAI = Time<TAIScale>`
  - `using TCG = Time<TCGScale>`
  - `using TCB = Time<TCBScale>`
  - `using GPS = Time<GPSScale>`
  - `using UT = Time<UTScale>`
  - `using UniversalTime = Time<UTScale>`
  - `using JDE = Time<JDEScale>`
  - `using UnixTime = Time<UnixTimeScale>`
- Local CI runner script (`run-ci.sh`) that executes lint, build, test, and coverage checks.
- Code coverage reporting to CI pipeline via gcovr (XML and HTML reports).
- `.gitignore` entries for build outputs and coverage artifacts.

### Changed

- Reorganized codebase to separate `CivilTime` definition into dedicated header (`civil_time.hpp`) to resolve circular dependencies.
- Refactored `scales.hpp` to include full `CivilTime` definition and improve template specialization organization.
- Updated `time_base.hpp` to use new `civil_time.hpp` header.

### Fixed

- Fixed clang-format violations in header files.
- Resolved incomplete type errors in clang-tidy by properly completing `CivilTime` definition before use in template specializations.
- Improved include chain to ensure all FFI-backed time scale conversions are properly typed and compiled.

## [0.1.0]

- Initial C++ wrapper for tempoch FFI library.
- Core `Time<S>` class template with FFI-backed time scale dispatch.
- Period arithmetic and interval operations.
- Example programs demonstrating time scale conversions and period operations.
