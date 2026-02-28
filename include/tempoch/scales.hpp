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

#include "ffi_core.hpp" // tempoch_ffi.h + check_status

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

/// Terrestrial Time (TT), stored as JD days in TT scale.
struct TTScale {};

/// International Atomic Time (TAI), stored as JD days in TAI scale.
struct TAIScale {};

/// Barycentric Dynamical Time (TDB), stored as JD days in TDB scale.
struct TDBScale {};

/// Geocentric Coordinate Time (TCG), stored as JD days in TCG scale.
struct TCGScale {};

/// Barycentric Coordinate Time (TCB), stored as JD days in TCB scale.
struct TCBScale {};

/// GPS Time, stored as JD days in GPS scale.
struct GPSScale {};

/// Universal Time (UT1), stored as JD days in UT1 scale.
struct UTScale {};

/// Julian Ephemeris Date (JDE ≡ TDB), stored as JD days.
struct JDEScale {};

/// Unix Time (seconds since 1970-01-01T00:00:00 UTC), stored as Unix seconds.
struct UnixTimeScale {};

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

  /// Difference as a typed `qtty_quantity_t` (Day unit).
  static qtty_quantity_t difference_qty(double a, double b) {
    return tempoch_jd_difference_qty(a, b);
  }

  /// Add a typed duration quantity and write result to @p out.
  static void add_qty(double jd, qtty_quantity_t duration, double &out) {
    check_status(tempoch_jd_add_qty(jd, duration, &out), "Time<JD>::add_qty");
  }

  /// J2000.0 epoch constant (JD 2 451 545.0).
  static double j2000() { return tempoch_jd_j2000(); }

  /// Julian centuries elapsed since J2000.
  static double julian_centuries(double jd) {
    return tempoch_jd_julian_centuries(jd);
  }

  /// Julian centuries since J2000 as a typed `qtty_quantity_t`
  /// (JulianCentury unit).
  static qtty_quantity_t julian_centuries_qty(double jd) {
    return tempoch_jd_julian_centuries_qty(jd);
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

  /// Difference as a typed `qtty_quantity_t` (Day unit).
  static qtty_quantity_t difference_qty(double a, double b) {
    return tempoch_mjd_difference_qty(a, b);
  }

  /// Add a typed duration quantity and write result to @p out.
  static void add_qty(double mjd, qtty_quantity_t duration, double &out) {
    check_status(tempoch_mjd_add_qty(mjd, duration, &out),
                 "Time<MJD>::add_qty");
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

  /// Difference as a typed `qtty_quantity_t` (Day unit).
  static qtty_quantity_t difference_qty(double a, double b) {
    return TimeScaleTraits<MJDScale>::difference_qty(a, b);
  }

  /// Add a typed duration quantity and write result to @p out.
  static void add_qty(double mjd, qtty_quantity_t duration, double &out) {
    TimeScaleTraits<MJDScale>::add_qty(mjd, duration, out);
  }
};

// ── JD-backed scale helper ───────────────────────────────────────────────────
// Scales stored as JD days in their own time-scale: arithmetic delegates
// through JD using the raw FFI functions.  from_civil / to_civil route via JD.

namespace detail {

/// Generic TimeScaleTraits for any scale S whose internal value is a JD-like
/// double, with jd_to_s / s_to_jd conversion functions.
/// from_civil / to_civil use deferred template parameters to avoid requiring
/// CivilTime to be complete at definition time.
template <typename S, double (*JdToS)(double), double (*SToJd)(double)>
struct JDBackedScaleTraits {
  /// CivilTime → JD → target scale.
  /// Template parameter CT defers instantiation until CivilTime is complete.
  template <typename CT = CivilTime>
  static double from_civil(const CT &ct) {
    double jd_val = TimeScaleTraits<JDScale>::from_civil(ct);
    return JdToS(jd_val);
  }

  /// Target scale → JD → CivilTime.
  template <typename CT = CivilTime>
  static CT to_civil(double val) {
    double jd_val = SToJd(val);
    return TimeScaleTraits<JDScale>::to_civil(jd_val);
  }

  static double add_days(double val, double delta) {
    // Convert to JD, add, convert back.
    double jd = SToJd(val);
    double jd_new = tempoch_jd_add_days(jd, delta);
    return JdToS(jd_new);
  }

  static double difference(double a, double b) {
    // Convert both to JD and subtract — preserves the day-count semantic.
    return tempoch_jd_difference(SToJd(a), SToJd(b));
  }

  static qtty_quantity_t difference_qty(double a, double b) {
    return tempoch_jd_difference_qty(SToJd(a), SToJd(b));
  }

  static void add_qty(double val, qtty_quantity_t duration, double &out) {
    double jd = SToJd(val);
    double jd_out;
    check_status(tempoch_jd_add_qty(jd, duration, &jd_out),
                 "Time<JD-backed>::add_qty");
    out = JdToS(jd_out);
  }
};

} // namespace detail

// ── TTScale ─────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<TTScale>
    : detail::JDBackedScaleTraits<TTScale, tempoch_jd_to_tt, tempoch_tt_to_jd> {
  static constexpr const char *label() { return "TT"; }
};

// ── TAIScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<TAIScale>
    : detail::JDBackedScaleTraits<TAIScale, tempoch_jd_to_tai, tempoch_tai_to_jd> {
  static constexpr const char *label() { return "TAI"; }
};

// ── TDBScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<TDBScale>
    : detail::JDBackedScaleTraits<TDBScale, tempoch_jd_to_tdb, tempoch_tdb_to_jd> {
  static constexpr const char *label() { return "TDB"; }
};

// ── TCGScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<TCGScale>
    : detail::JDBackedScaleTraits<TCGScale, tempoch_jd_to_tcg, tempoch_tcg_to_jd> {
  static constexpr const char *label() { return "TCG"; }
};

// ── TCBScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<TCBScale>
    : detail::JDBackedScaleTraits<TCBScale, tempoch_jd_to_tcb, tempoch_tcb_to_jd> {
  static constexpr const char *label() { return "TCB"; }
};

// ── GPSScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<GPSScale>
    : detail::JDBackedScaleTraits<GPSScale, tempoch_jd_to_gps, tempoch_gps_to_jd> {
  static constexpr const char *label() { return "GPS"; }
};

// ── UTScale ─────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<UTScale>
    : detail::JDBackedScaleTraits<UTScale, tempoch_jd_to_ut, tempoch_ut_to_jd> {
  static constexpr const char *label() { return "UT1"; }
};

// ── JDEScale ────────────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<JDEScale>
    : detail::JDBackedScaleTraits<JDEScale, tempoch_jd_to_jde, tempoch_jde_to_jd> {
  static constexpr const char *label() { return "JDE"; }
};

// ── UnixTimeScale ───────────────────────────────────────────────────────────

template <> struct TimeScaleTraits<UnixTimeScale>
    : detail::JDBackedScaleTraits<UnixTimeScale, tempoch_jd_to_unix, tempoch_unix_to_jd> {
  static constexpr const char *label() { return "Unix"; }
};

// ============================================================================
// TimeConvertTraits — cross-scale conversion
// ============================================================================

/**
 * @brief Primary template — routes any A→B conversion through JD.
 *
 * Explicit specialisations for direct pairs (e.g. JD↔MJD, JD↔TDB)
 * bypass this fallback.  For all other pairs, the default implementation
 * computes A → JD → B using the two explicitly-specialised half-paths.
 */
template <typename From, typename To> struct TimeConvertTraits {
  static double convert(double src) {
    double jd = TimeConvertTraits<From, JDScale>::convert(src);
    return TimeConvertTraits<JDScale, To>::convert(jd);
  }
};

/// Identity conversion — zero cost.
template <typename S> struct TimeConvertTraits<S, S> {
  static double convert(double v) { return v; }
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

// ── JD ↔ TT ────────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, TTScale> {
  static double convert(double jd) { return tempoch_jd_to_tt(jd); }
};
template <> struct TimeConvertTraits<TTScale, JDScale> {
  static double convert(double tt) { return tempoch_tt_to_jd(tt); }
};

// ── JD ↔ TAI ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, TAIScale> {
  static double convert(double jd) { return tempoch_jd_to_tai(jd); }
};
template <> struct TimeConvertTraits<TAIScale, JDScale> {
  static double convert(double tai) { return tempoch_tai_to_jd(tai); }
};

// ── JD ↔ TDB ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, TDBScale> {
  static double convert(double jd) { return tempoch_jd_to_tdb(jd); }
};
template <> struct TimeConvertTraits<TDBScale, JDScale> {
  static double convert(double tdb) { return tempoch_tdb_to_jd(tdb); }
};

// ── JD ↔ TCG ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, TCGScale> {
  static double convert(double jd) { return tempoch_jd_to_tcg(jd); }
};
template <> struct TimeConvertTraits<TCGScale, JDScale> {
  static double convert(double tcg) { return tempoch_tcg_to_jd(tcg); }
};

// ── JD ↔ TCB ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, TCBScale> {
  static double convert(double jd) { return tempoch_jd_to_tcb(jd); }
};
template <> struct TimeConvertTraits<TCBScale, JDScale> {
  static double convert(double tcb) { return tempoch_tcb_to_jd(tcb); }
};

// ── JD ↔ GPS ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, GPSScale> {
  static double convert(double jd) { return tempoch_jd_to_gps(jd); }
};
template <> struct TimeConvertTraits<GPSScale, JDScale> {
  static double convert(double gps) { return tempoch_gps_to_jd(gps); }
};

// ── JD ↔ UT ────────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, UTScale> {
  static double convert(double jd) { return tempoch_jd_to_ut(jd); }
};
template <> struct TimeConvertTraits<UTScale, JDScale> {
  static double convert(double ut) { return tempoch_ut_to_jd(ut); }
};

// ── JD ↔ JDE ───────────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, JDEScale> {
  static double convert(double jd) { return tempoch_jd_to_jde(jd); }
};
template <> struct TimeConvertTraits<JDEScale, JDScale> {
  static double convert(double jde) { return tempoch_jde_to_jd(jde); }
};

// ── JD ↔ UnixTime ──────────────────────────────────────────────────────────

template <> struct TimeConvertTraits<JDScale, UnixTimeScale> {
  static double convert(double jd) { return tempoch_jd_to_unix(jd); }
};
template <> struct TimeConvertTraits<UnixTimeScale, JDScale> {
  static double convert(double unix_t) { return tempoch_unix_to_jd(unix_t); }
};

// ── Generic cross-scale ────────────────────────────────────────────────────
// For any two non-JD scales A↔B, route through JD:  A → JD → B.
// (MJD/UTC conversions are handled by explicit specializations above.)

template <> struct TimeConvertTraits<MJDScale, TTScale> {
  static double convert(double m) { return tempoch_jd_to_tt(tempoch_mjd_to_jd(m)); }
};
template <> struct TimeConvertTraits<TTScale, MJDScale> {
  static double convert(double t) { return tempoch_jd_to_mjd(tempoch_tt_to_jd(t)); }
};

template <> struct TimeConvertTraits<MJDScale, TDBScale> {
  static double convert(double m) { return tempoch_jd_to_tdb(tempoch_mjd_to_jd(m)); }
};
template <> struct TimeConvertTraits<TDBScale, MJDScale> {
  static double convert(double t) { return tempoch_jd_to_mjd(tempoch_tdb_to_jd(t)); }
};

template <> struct TimeConvertTraits<MJDScale, TAIScale> {
  static double convert(double m) { return tempoch_jd_to_tai(tempoch_mjd_to_jd(m)); }
};
template <> struct TimeConvertTraits<TAIScale, MJDScale> {
  static double convert(double t) { return tempoch_jd_to_mjd(tempoch_tai_to_jd(t)); }
};

} // namespace tempoch
