#pragma once

/**
 * @file time.hpp
 * @brief Public type aliases for tempoch time types.
 *
 * All implementation lives in `time_base.hpp` (the `Time<S>` class template)
 * and `scales.hpp` (scale tags and traits).  This header provides the
 * backward-compatible names that the rest of the codebase expects:
 *
 *   - `tempoch::JulianDate`  = `Time<scales::JD>`
 *   - `tempoch::MJD`         = `Time<scales::MJD>`
 *   - `tempoch::UTC`         = `CivilTime` (civil date-time breakdown)
 *   - `tempoch::CivilTime`   (canonical name for the civil struct)
 */

#include "time_base.hpp"

namespace tempoch {

/// Julian Date — days since −4712-01-01T12:00 TT.
using JulianDate = Time<scales::JD>;

/// Modified Julian Date — JD − 2 400 000.5.
using MJD = Time<scales::MJD>;

/// Barycentric Dynamical Time.
using TDB = Time<scales::TDB>;

/// Terrestrial Time.
using TT = Time<scales::TT>;

/// International Atomic Time.
using TAI = Time<scales::TAI>;

/// Geocentric Coordinate Time.
using TCG = Time<scales::TCG>;

/// Barycentric Coordinate Time.
using TCB = Time<scales::TCB>;

/// GPS Time.
using GPS = Time<scales::GPS>;

/// Universal Time (UT1).
using UT = Time<scales::UT>;

/// Alias — mirrors Rust's `UniversalTime`.
using UniversalTime = Time<scales::UT>;

/// Julian Ephemeris Date (≡ TDB expressed as JD).
using JDE = Time<scales::JDE>;

/// Unix (POSIX) time — seconds since 1970-01-01T00:00:00 UTC.
using UnixTime = Time<scales::Unix>;

// `UTC` and `CivilTime` are already declared in time_base.hpp:
//   using UTC = CivilTime;

} // namespace tempoch
