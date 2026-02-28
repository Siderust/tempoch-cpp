#pragma once

/**
 * @file time.hpp
 * @brief Public type aliases for tempoch time types.
 *
 * All implementation lives in `time_base.hpp` (the `Time<S>` class template)
 * and `scales.hpp` (scale tags and traits).  This header provides the
 * backward-compatible names that the rest of the codebase expects:
 *
 *   - `tempoch::JulianDate`  = `Time<JDScale>`
 *   - `tempoch::MJD`         = `Time<MJDScale>`
 *   - `tempoch::UTC`         = `CivilTime` (civil date-time breakdown)
 *   - `tempoch::CivilTime`   (canonical name for the civil struct)
 */

#include "time_base.hpp"

namespace tempoch {

/// Julian Date — days since −4712-01-01T12:00 TT.
using JulianDate = Time<JDScale>;

/// Modified Julian Date — JD − 2 400 000.5.
using MJD = Time<MJDScale>;

/// Barycentric Dynamical Time.
using TDB = Time<TDBScale>;

/// Terrestrial Time.
using TT = Time<TTScale>;

/// International Atomic Time.
using TAI = Time<TAIScale>;

/// Geocentric Coordinate Time.
using TCG = Time<TCGScale>;

/// Barycentric Coordinate Time.
using TCB = Time<TCBScale>;

/// GPS Time.
using GPS = Time<GPSScale>;

/// Universal Time (UT1).
using UT = Time<UTScale>;

/// Alias — mirrors Rust's `UniversalTime`.
using UniversalTime = Time<UTScale>;

/// Julian Ephemeris Date (≡ TDB expressed as JD).
using JDE = Time<JDEScale>;

/// Unix (POSIX) time — seconds since 1970-01-01T00:00:00 UTC.
using UnixTime = Time<UnixTimeScale>;

// `UTC` and `CivilTime` are already declared in time_base.hpp:
//   using UTC = CivilTime;

} // namespace tempoch
