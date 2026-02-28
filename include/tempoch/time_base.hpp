#pragma once

/**
 * @file time_base.hpp
 * @brief Core `Time<S>` class template and `CivilTime` struct.
 *
 * Mirrors the Rust `tempoch_core::instant::Time<S: TimeScale>` design:
 *   - A single `double` (days in the scale's epoch) with compile-time scale
 *     dispatch via `TimeScaleTraits<S>`.
 *   - Cross-scale `.to<T>()` via `TimeConvertTraits<S,T>`.
 *   - `CivilTime` replaces the old `UTC` aggregate struct.
 *   - JD-specific extras (`J2000()`, `julian_centuries()`) are enabled only
 *     for `Time<JDScale>` via SFINAE.
 */

#include "civil_time.hpp"
#include "qtty/qtty.hpp"
#include "scales.hpp"
#include <iomanip>
#include <ostream>
#include <type_traits>

namespace tempoch {

// ============================================================================
// TimeScaleTraits deferred implementations (need CivilTime to be complete)
// ============================================================================

inline double TimeScaleTraits<JDScale>::from_civil(const CivilTime &ct) {
  double jd;
  auto c = ct.to_c();
  check_status(tempoch_jd_from_utc(c, &jd), "Time<JD>::from_utc");
  return jd;
}

inline CivilTime TimeScaleTraits<JDScale>::to_civil(double jd) {
  tempoch_utc_t out;
  check_status(tempoch_jd_to_utc(jd, &out), "Time<JD>::to_utc");
  return CivilTime::from_c(out);
}

inline double TimeScaleTraits<MJDScale>::from_civil(const CivilTime &ct) {
  double mjd;
  auto c = ct.to_c();
  check_status(tempoch_mjd_from_utc(c, &mjd), "Time<MJD>::from_utc");
  return mjd;
}

inline CivilTime TimeScaleTraits<MJDScale>::to_civil(double mjd) {
  tempoch_utc_t out;
  check_status(tempoch_mjd_to_utc(mjd, &out), "Time<MJD>::to_utc");
  return CivilTime::from_c(out);
}

// UTCScale delegates to MJDScale (same internal representation).
inline double TimeScaleTraits<UTCScale>::from_civil(const CivilTime &ct) {
  return TimeScaleTraits<MJDScale>::from_civil(ct);
}

inline CivilTime TimeScaleTraits<UTCScale>::to_civil(double mjd) {
  return TimeScaleTraits<MJDScale>::to_civil(mjd);
}

// ============================================================================
// Time<S> — the core template
// ============================================================================

/**
 * @brief A point in time on scale @p S, stored as a raw `double` (days).
 *
 * Mirrors `tempoch_core::instant::Time<S: TimeScale>`.  Most operations are
 * dispatched through `TimeScaleTraits<S>`, keeping this class small and
 * reusable across all scales.
 *
 * @tparam S A scale tag for which `TimeScaleTraits<S>` is specialised.
 *
 * @code
 * using JulianDate = tempoch::Time<tempoch::JDScale>;
 * auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
 * auto mjd = jd.to<tempoch::MJDScale>();
 * @endcode
 */
template <typename S> class Time {
  double m_days;

public:
  using scale_type = S;

  // ── Constructors ──────────────────────────────────────────────────────

  /// Construct from a raw day count in this scale.
  constexpr explicit Time(double days) : m_days(days) {}

  /// Default-construct to zero (MJD epoch).
  constexpr Time() : m_days(0.0) {}

  // ── Factory: from civil time ──────────────────────────────────────────

  /**
   * @brief Create from a UTC civil-time breakdown.
   *
   * Accepts brace-initialised `CivilTime`, e.g.:
   * @code
   * auto jd = JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
   * @endcode
   */
  static Time from_utc(const CivilTime &ct) {
    return Time(TimeScaleTraits<S>::from_civil(ct));
  }

  // ── Accessors ─────────────────────────────────────────────────────────

  /// Raw value in this scale's day-count.
  constexpr double value() const { return m_days; }

  /// Human-readable label for the scale (e.g. "JD", "MJD", "UTC").
  static constexpr const char *label() { return TimeScaleTraits<S>::label(); }

  // ── Civil-time conversion ─────────────────────────────────────────────

  /// Convert to a UTC civil-time breakdown.
  CivilTime to_utc() const { return TimeScaleTraits<S>::to_civil(m_days); }

  // ── Cross-scale conversion ────────────────────────────────────────────

  /**
   * @brief Convert to another time scale.
   * @tparam T Target scale tag (e.g. `MJDScale`).
   */
  template <typename T> Time<T> to() const {
    return Time<T>(TimeConvertTraits<S, T>::convert(m_days));
  }

  // ── Arithmetic ────────────────────────────────────────────────────────

  /**
   * @brief Advance by a typed time quantity.
   *
   * Accepts any qtty time unit; the value is converted to days internally.
   * @code
   * auto t2 = jd  + qtty::Day(365.25);
   * auto t3 = mjd + qtty::Hour(12.0);
   * auto t4 = mjd + 30.0_min;            // using qtty::literals
   * @endcode
   */
  template <typename Q> Time operator+(const qtty::Quantity<Q> &delta) const {
    qtty_quantity_t qty{delta.value(), qtty::UnitTraits<Q>::unit_id()};
    double result;
    TimeScaleTraits<S>::add_qty(m_days, qty, result);
    return Time(result);
  }

  /**
   * @brief Retreat by a typed time quantity.
   */
  template <typename Q> Time operator-(const qtty::Quantity<Q> &delta) const {
    qtty_quantity_t neg_qty{-delta.value(), qtty::UnitTraits<Q>::unit_id()};
    double result;
    TimeScaleTraits<S>::add_qty(m_days, neg_qty, result);
    return Time(result);
  }

  /**
   * @brief Elapsed duration between two instants, returned as `qtty::Day`.
   *
   * Convert to other units with `.to<qtty::Hour>()` etc.
   * @code
   * qtty::Day  d = t2 - t1;
   * qtty::Hour h = (t2 - t1).to<qtty::Hour>();
   * @endcode
   */
  qtty::Day operator-(const Time &other) const {
    auto qty = TimeScaleTraits<S>::difference_qty(m_days, other.m_days);
    return qtty::Day(qty.value);
  }

  // ── Comparisons ───────────────────────────────────────────────────────

  bool operator==(const Time &o) const { return m_days == o.m_days; }
  bool operator!=(const Time &o) const { return m_days != o.m_days; }
  bool operator<(const Time &o) const { return m_days < o.m_days; }
  bool operator<=(const Time &o) const { return m_days <= o.m_days; }
  bool operator>(const Time &o) const { return m_days > o.m_days; }
  bool operator>=(const Time &o) const { return m_days >= o.m_days; }

  // ── JD-only extras (SFINAE-guarded) ───────────────────────────────────

  /// J2000.0 epoch (JD 2 451 545.0).  Only available for `Time<JDScale>`.
  template <typename U = S>
  static std::enable_if_t<std::is_same_v<U, JDScale>, Time> J2000() {
    return Time(TimeScaleTraits<JDScale>::j2000());
  }

  /// Julian centuries since J2000.  Only available for `Time<JDScale>`.
  template <typename U = S>
  std::enable_if_t<std::is_same_v<U, JDScale>, double>
  julian_centuries() const {
    return TimeScaleTraits<JDScale>::julian_centuries(m_days);
  }

  /// Julian centuries since J2000 as a typed quantity.
  /// Only for `Time<JDScale>`.
  template <typename U = S>
  std::enable_if_t<std::is_same_v<U, JDScale>, qtty::JulianCentury>
  julian_centuries_qty() const {
    auto qty = TimeScaleTraits<JDScale>::julian_centuries_qty(m_days);
    return qtty::JulianCentury(qty.value);
  }

  // ── JD ↔ MJD convenience (preserves old API surface) ──────────────────

  /// Convert to MJD double.  Only available for `Time<JDScale>`.
  template <typename U = S>
  std::enable_if_t<std::is_same_v<U, JDScale>, double> to_mjd() const {
    return TimeConvertTraits<JDScale, MJDScale>::convert(m_days);
  }

  /// Create from a JulianDate.  Only available for `Time<MJDScale>`.
  template <typename U = S>
  static std::enable_if_t<std::is_same_v<U, MJDScale>, Time>
  from_jd(const Time<JDScale> &jd) {
    return Time(TimeConvertTraits<JDScale, MJDScale>::convert(jd.value()));
  }

  /// Convert to JulianDate.  Only available for `Time<MJDScale>`.
  template <typename U = S>
  std::enable_if_t<std::is_same_v<U, MJDScale>, Time<JDScale>> to_jd() const {
    return Time<JDScale>(TimeConvertTraits<MJDScale, JDScale>::convert(m_days));
  }

  // ── UT-only extras (SFINAE-guarded) ───────────────────────────────────

  /// ΔT = TT − UT1 in seconds.  Only available for `Time<UTScale>`.
  template <typename U = S>
  std::enable_if_t<std::is_same_v<U, UTScale>, qtty::Second> delta_t() const {
    double jd = TimeConvertTraits<UTScale, JDScale>::convert(m_days);
    return qtty::Second(tempoch_delta_t_seconds(jd));
  }
};

// ============================================================================
// operator<<  —  streams "<label>:<value>"
// ============================================================================

template <typename S>
inline std::ostream &operator<<(std::ostream &os, const Time<S> &t) {
  return os << t.value();
}

// ============================================================================
// TimeTraits<Time<S>>  —  connects Time<S> to Period<T>
// ============================================================================

// Forward declaration — full Period template lives in period.hpp.
template <typename T> struct TimeTraits;

/// Generic TimeTraits for any Time<S>.  Converts via MJD internally.
template <typename S> struct TimeTraits<Time<S>> {
  static double to_mjd_value(const Time<S> &t) {
    if constexpr (std::is_same_v<S, MJDScale> || std::is_same_v<S, UTCScale>) {
      return t.value();
    } else {
      return t.template to<MJDScale>().value();
    }
  }

  static Time<S> from_mjd_value(double mjd) {
    if constexpr (std::is_same_v<S, MJDScale> || std::is_same_v<S, UTCScale>) {
      return Time<S>(mjd);
    } else {
      return Time<MJDScale>(mjd).template to<S>();
    }
  }
};

// ============================================================================
// Backward-compatible alias
// ============================================================================

/// Old name kept for source compatibility.
using UTC = CivilTime;

} // namespace tempoch
