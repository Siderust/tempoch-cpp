# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
