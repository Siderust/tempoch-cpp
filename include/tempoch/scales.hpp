#pragma once

/**
 * @file scales.hpp
 * @brief Time-scale tag types and traits for the tempoch Time<S> template.
 *
 * Mirrors the Rust scale set while keeping the C++ API strongly typed.
 * The C ABI underneath is intentionally simpler: all generic dispatch goes
 * through `double` values plus a raw scale id.
 */

#include "civil_time.hpp" // CivilTime struct definition
#include "ffi_core.hpp"   // tempoch_ffi.h + check_status
#include <cstdint>

namespace tempoch {

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

namespace detail {

template <typename S> struct ScaleIdOf;

template <> struct ScaleIdOf<JDScale> {
  static constexpr int32_t value = 0;
};
template <> struct ScaleIdOf<MJDScale> {
  static constexpr int32_t value = 1;
};
// UTC is stored as MJD days in the FFI layer (TempochScaleId::MJD = 1).
// tempoch_time_from_utc/to_utc with scale_id=1 perform the UTC↔civil
// conversion. This aliasing is intentional: UTC and MJD share the same
// underlying numeric representation in the C ABI.
template <> struct ScaleIdOf<UTCScale> {
  static constexpr int32_t value = 1;
};
template <> struct ScaleIdOf<TDBScale> {
  static constexpr int32_t value = 2;
};
template <> struct ScaleIdOf<TTScale> {
  static constexpr int32_t value = 3;
};
template <> struct ScaleIdOf<TAIScale> {
  static constexpr int32_t value = 4;
};
template <> struct ScaleIdOf<TCGScale> {
  static constexpr int32_t value = 5;
};
template <> struct ScaleIdOf<TCBScale> {
  static constexpr int32_t value = 6;
};
template <> struct ScaleIdOf<GPSScale> {
  static constexpr int32_t value = 7;
};
template <> struct ScaleIdOf<UTScale> {
  static constexpr int32_t value = 8;
};
template <> struct ScaleIdOf<JDEScale> {
  static constexpr int32_t value = 9;
};
template <> struct ScaleIdOf<UnixTimeScale> {
  static constexpr int32_t value = 10;
};

template <typename S> inline constexpr int32_t scale_id_v = ScaleIdOf<S>::value;

inline double time_from_utc(const CivilTime &ct, int32_t scale_id,
                            const char *operation) {
  double out = 0.0;
  auto raw = ct.to_c();
  check_status(tempoch_time_from_utc(raw, scale_id, &out), operation);
  return out;
}

inline CivilTime time_to_utc(double value, int32_t scale_id,
                             const char *operation) {
  tempoch_utc_t out{};
  check_status(tempoch_time_to_utc(value, scale_id, &out), operation);
  return CivilTime::from_c(out);
}

inline double time_add_days(double value, int32_t scale_id, double delta,
                            const char *operation) {
  double out = 0.0;
  check_status(tempoch_time_add_days(value, scale_id, delta, &out), operation);
  return out;
}

inline double time_difference_days(double lhs, double rhs, int32_t scale_id,
                                   const char *operation) {
  double out = 0.0;
  check_status(tempoch_time_difference_days(lhs, rhs, scale_id, &out),
               operation);
  return out;
}

inline qtty_quantity_t time_difference_qty(double lhs, double rhs,
                                           int32_t scale_id,
                                           const char *operation) {
  qtty_quantity_t out{0.0, UNIT_ID_DAY};
  check_status(tempoch_time_difference_qty(lhs, rhs, scale_id, &out),
               operation);
  return out;
}

inline void time_add_qty(double value, int32_t scale_id,
                         qtty_quantity_t duration, double &out,
                         const char *operation) {
  check_status(tempoch_time_add_qty(value, scale_id, duration, &out),
               operation);
}

template <typename S> struct GenericScaleTraits {
  static double from_civil(const CivilTime &ct) {
    return time_from_utc(ct, scale_id_v<S>, "tempoch_time_from_utc");
  }

  static CivilTime to_civil(double value) {
    return time_to_utc(value, scale_id_v<S>, "tempoch_time_to_utc");
  }

  static double add_days(double value, double delta) {
    return time_add_days(value, scale_id_v<S>, delta, "tempoch_time_add_days");
  }

  static double difference(double a, double b) {
    return time_difference_days(a, b, scale_id_v<S>,
                                "tempoch_time_difference_days");
  }

  static qtty_quantity_t difference_qty(double a, double b) {
    return time_difference_qty(a, b, scale_id_v<S>,
                               "tempoch_time_difference_qty");
  }

  static void add_qty(double value, qtty_quantity_t duration, double &out) {
    time_add_qty(value, scale_id_v<S>, duration, out, "tempoch_time_add_qty");
  }
};

} // namespace detail

template <>
struct TimeScaleTraits<JDScale> : detail::GenericScaleTraits<JDScale> {
  static constexpr const char *label() { return "JD"; }

  static double j2000() { return tempoch_jd_j2000(); }

  static double julian_centuries(double jd) {
    return tempoch_jd_julian_centuries(jd);
  }

  static qtty_quantity_t julian_centuries_qty(double jd) {
    return tempoch_jd_julian_centuries_qty(jd);
  }
};

template <>
struct TimeScaleTraits<MJDScale> : detail::GenericScaleTraits<MJDScale> {
  static constexpr const char *label() { return "MJD"; }
};

template <>
struct TimeScaleTraits<UTCScale> : detail::GenericScaleTraits<UTCScale> {
  static constexpr const char *label() { return "UTC"; }
};

template <>
struct TimeScaleTraits<TTScale> : detail::GenericScaleTraits<TTScale> {
  static constexpr const char *label() { return "TT"; }
};

template <>
struct TimeScaleTraits<TAIScale> : detail::GenericScaleTraits<TAIScale> {
  static constexpr const char *label() { return "TAI"; }
};

template <>
struct TimeScaleTraits<TDBScale> : detail::GenericScaleTraits<TDBScale> {
  static constexpr const char *label() { return "TDB"; }
};

template <>
struct TimeScaleTraits<TCGScale> : detail::GenericScaleTraits<TCGScale> {
  static constexpr const char *label() { return "TCG"; }
};

template <>
struct TimeScaleTraits<TCBScale> : detail::GenericScaleTraits<TCBScale> {
  static constexpr const char *label() { return "TCB"; }
};

template <>
struct TimeScaleTraits<GPSScale> : detail::GenericScaleTraits<GPSScale> {
  static constexpr const char *label() { return "GPS"; }
};

template <>
struct TimeScaleTraits<UTScale> : detail::GenericScaleTraits<UTScale> {
  static constexpr const char *label() { return "UT1"; }
};

template <>
struct TimeScaleTraits<JDEScale> : detail::GenericScaleTraits<JDEScale> {
  static constexpr const char *label() { return "JDE"; }
};

template <>
struct TimeScaleTraits<UnixTimeScale>
    : detail::GenericScaleTraits<UnixTimeScale> {
  static constexpr const char *label() { return "Unix"; }
};

// ============================================================================
// TimeConvertTraits — cross-scale conversion
// ============================================================================

/**
 * @brief Primary template — routes any A→B conversion through JD.
 *
 * All cross-scale conversion goes through the generic `tempoch_time_convert`
 * ABI entrypoint and compile-time scale-id mapping.
 */
template <typename From, typename To> struct TimeConvertTraits {
  static double convert(double src) {
    double out = 0.0;
    check_status(tempoch_time_convert(src, detail::scale_id_v<From>,
                                      detail::scale_id_v<To>, &out),
                 "tempoch_time_convert");
    return out;
  }
};

/// Identity conversion — zero cost.
template <typename S> struct TimeConvertTraits<S, S> {
  static double convert(double v) { return v; }
};

} // namespace tempoch
