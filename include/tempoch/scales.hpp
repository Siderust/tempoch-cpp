#pragma once

/**
 * @file scales.hpp
 * @brief Time-scale tag types and traits for the tempoch Time<S> template.
 *
 * Scale tag structs live in `tempoch::scales::` (e.g., `scales::TT`,
 * `scales::JD`).  Convenience type aliases (`TT = Time<scales::TT>` etc.)
 * are provided in `time.hpp`.
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
// Scale Tags — in tempoch::scales::
// ============================================================================

namespace scales {

/// Julian Date (days since −4712‑01‑01T12:00 TT).
struct JD {};

/// Modified Julian Date (JD − 2 400 000.5).
struct MJD {};

/// UTC, internally stored as MJD days for arithmetic.
struct UTC {};

/// Terrestrial Time (TT), stored as JD days in TT scale.
struct TT {};

/// International Atomic Time (TAI), stored as JD days in TAI scale.
struct TAI {};

/// Barycentric Dynamical Time (TDB), stored as JD days in TDB scale.
struct TDB {};

/// Geocentric Coordinate Time (TCG), stored as JD days in TCG scale.
struct TCG {};

/// Barycentric Coordinate Time (TCB), stored as JD days in TCB scale.
struct TCB {};

/// GPS Time, stored as JD days in GPS scale.
struct GPS {};

/// Universal Time (UT1), stored as JD days in UT1 scale.
struct UT {};

/// Julian Ephemeris Date (JDE ≡ TDB), stored as JD days.
struct JDE {};

/// Unix Time (seconds since 1970-01-01T00:00:00 UTC), stored as Unix seconds.
struct Unix {};

} // namespace scales

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

template <> struct ScaleIdOf<scales::JD> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_JD;
};
template <> struct ScaleIdOf<scales::MJD> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_MJD;
};
// UTC is stored as MJD days in the FFI layer (TempochScaleId::MJD = 1).
// tempoch_time_from_utc/to_utc with scale_id=1 perform the UTC↔civil
// conversion. This aliasing is intentional: UTC and MJD share the same
// underlying numeric representation in the C ABI.
template <> struct ScaleIdOf<scales::UTC> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_MJD;
};
template <> struct ScaleIdOf<scales::TDB> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_TDB;
};
template <> struct ScaleIdOf<scales::TT> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_TT;
};
template <> struct ScaleIdOf<scales::TAI> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_TAI;
};
template <> struct ScaleIdOf<scales::TCG> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_TCG;
};
template <> struct ScaleIdOf<scales::TCB> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_TCB;
};
template <> struct ScaleIdOf<scales::GPS> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_GPS;
};
template <> struct ScaleIdOf<scales::UT> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_UT;
};
template <> struct ScaleIdOf<scales::JDE> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_JDE;
};
template <> struct ScaleIdOf<scales::Unix> {
  static constexpr int32_t value = TEMPOCH_SCALE_ID_T_UNIX_TIME;
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
struct TimeScaleTraits<scales::JD> : detail::GenericScaleTraits<scales::JD> {
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
struct TimeScaleTraits<scales::MJD> : detail::GenericScaleTraits<scales::MJD> {
  static constexpr const char *label() { return "MJD"; }
};

template <>
struct TimeScaleTraits<scales::UTC> : detail::GenericScaleTraits<scales::UTC> {
  static constexpr const char *label() { return "UTC"; }
};

template <>
struct TimeScaleTraits<scales::TT> : detail::GenericScaleTraits<scales::TT> {
  static constexpr const char *label() { return "TT"; }
};

template <>
struct TimeScaleTraits<scales::TAI> : detail::GenericScaleTraits<scales::TAI> {
  static constexpr const char *label() { return "TAI"; }
};

template <>
struct TimeScaleTraits<scales::TDB> : detail::GenericScaleTraits<scales::TDB> {
  static constexpr const char *label() { return "TDB"; }
};

template <>
struct TimeScaleTraits<scales::TCG> : detail::GenericScaleTraits<scales::TCG> {
  static constexpr const char *label() { return "TCG"; }
};

template <>
struct TimeScaleTraits<scales::TCB> : detail::GenericScaleTraits<scales::TCB> {
  static constexpr const char *label() { return "TCB"; }
};

template <>
struct TimeScaleTraits<scales::GPS> : detail::GenericScaleTraits<scales::GPS> {
  static constexpr const char *label() { return "GPS"; }
};

template <>
struct TimeScaleTraits<scales::UT> : detail::GenericScaleTraits<scales::UT> {
  static constexpr const char *label() { return "UT1"; }
};

template <>
struct TimeScaleTraits<scales::JDE> : detail::GenericScaleTraits<scales::JDE> {
  static constexpr const char *label() { return "JDE"; }
};

template <>
struct TimeScaleTraits<scales::Unix>
    : detail::GenericScaleTraits<scales::Unix> {
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
