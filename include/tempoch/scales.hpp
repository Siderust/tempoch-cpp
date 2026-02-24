#pragma once

/**
 * @file scales.hpp
 * @brief Time-scale tag types and traits for the tempoch Time<S> template.
 *
 * Mirrors the Rust `tempoch_core::scales` module.  Each tag is an empty struct
 * that selects the FFI functions used by `Time<S>`.
 *
 * Adding a new scale requires:
 *  1. Define a tag struct (e.g. `struct TTScale {};`).
 *  2. Specialise `TimeScaleTraits<TTScale>` with the FFI calls.
 *  3. Specialise `TimeConvertTraits<A,B>` for each supported conversion pair.
 */

#include "ffi_core.hpp"  // tempoch_ffi.h + check_status

namespace tempoch {

// Forward declaration — defined in time_base.hpp.
struct CivilTime;

// ============================================================================
// Scale Tags
// ============================================================================

/// Julian Date (days since −4712‑01‑01T12:00 TT).
struct JDScale {};

/// Modified Julian Date (JD − 2 400 000.5).
struct MJDScale {};

/// UTC, internally stored as MJD days for arithmetic.
struct UTCScale {};

// Stubs for future FFI-backed scales — uncomment and specialise traits once
// the FFI exposes the conversion functions.
// struct TTScale  {};   // Terrestrial Time
// struct TAIScale {};   // International Atomic Time
// struct TDBScale {};   // Barycentric Dynamical Time
// struct TCGScale {};   // Geocentric Coordinate Time
// struct TCBScale {};   // Barycentric Coordinate Time
// struct GPSScale {};   // GPS Time
// struct UTScale  {};   // Universal Time (UT1)

// ============================================================================
// TimeScaleTraits — per-scale FFI dispatch
// ============================================================================

/**
 * @brief Primary template — must be specialised for every supported scale.
 *
 * Required static members:
 *   - `const char* label()`
 *   - `double from_utc(const CivilTime&)` — civil time → raw days
 *   - `CivilTime to_utc(double days)` — raw days → civil time
 *   - `double add_days(double days, double delta)`
 *   - `double difference(double a, double b)` — a − b in days
 */
template <typename S> struct TimeScaleTraits {
  static_assert(sizeof(S) == 0,
                "TimeScaleTraits<S> must be specialised for this scale.");
};

// ── JDScale ─────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<JDScale> {
  static constexpr const char *label() { return "JD"; }

  static double from_civil(const CivilTime &ct);
  static CivilTime to_civil(double jd);

  static double add_days(double jd, double delta) {
    return tempoch_jd_add_days(jd, delta);
  }

  static double difference(double a, double b) {
    return tempoch_jd_difference(a, b);
  }

  /// J2000.0 epoch constant (JD 2 451 545.0).
  static double j2000() { return tempoch_jd_j2000(); }

  /// Julian centuries elapsed since J2000.
  static double julian_centuries(double jd) {
    return tempoch_jd_julian_centuries(jd);
  }
};

// ── MJDScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<MJDScale> {
  static constexpr const char *label() { return "MJD"; }

  static double from_civil(const CivilTime &ct);
  static CivilTime to_civil(double mjd);

  static double add_days(double mjd, double delta) {
    return tempoch_mjd_add_days(mjd, delta);
  }

  static double difference(double a, double b) {
    return tempoch_mjd_difference(a, b);
  }
};

// ── UTCScale (internally stored as MJD) ─────────────────────────────────────

template <> struct TimeScaleTraits<UTCScale> {
  static constexpr const char *label() { return "UTC"; }

  static double from_civil(const CivilTime &ct);
  static CivilTime to_civil(double mjd);

  static double add_days(double mjd, double delta) {
    return tempoch_mjd_add_days(mjd, delta);
  }

  static double difference(double a, double b) {
    return tempoch_mjd_difference(a, b);
  }
};

// ============================================================================
// TimeConvertTraits — cross-scale conversion
// ============================================================================

/**
 * @brief Primary template — specialise for each supported A→B pair.
 *
 * Required: `static double convert(double src_days)`.
 */
template <typename From, typename To> struct TimeConvertTraits {
  static_assert(sizeof(From) == 0,
                "TimeConvertTraits<From,To> is not specialised for this pair.");
};

// ── JD ↔ MJD ────────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, MJDScale> {
  static double convert(double jd) { return tempoch_jd_to_mjd(jd); }
};

template <> struct TimeConvertTraits<MJDScale, JDScale> {
  static double convert(double mjd) { return tempoch_mjd_to_jd(mjd); }
};

// ── JD ↔ UTC (UTC stored as MJD internally) ────────────────────────────────

template <> struct TimeConvertTraits<JDScale, UTCScale> {
  static double convert(double jd) { return tempoch_jd_to_mjd(jd); }
};

template <> struct TimeConvertTraits<UTCScale, JDScale> {
  static double convert(double mjd) { return tempoch_mjd_to_jd(mjd); }
};

// ── MJD ↔ UTC (identity — both stored as MJD) ──────────────────────────────

template <> struct TimeConvertTraits<MJDScale, UTCScale> {
  static double convert(double mjd) { return mjd; }
};

template <> struct TimeConvertTraits<UTCScale, MJDScale> {
  static double convert(double mjd) { return mjd; }
};

} // namespace tempoch
