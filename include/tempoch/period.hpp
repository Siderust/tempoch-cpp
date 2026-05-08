#pragma once

/**
 * @file period.hpp
 * @brief C++ wrapper for time periods, generic over any time type.
 *
 * Wraps the tempoch-ffi Period API with a value-semantic, exception-safe C++
 * class template `Period<T>`.  The underlying storage is always
 * `tempoch_period_mjd_t`; `TimeTraits<T>` handles conversion to/from the raw
 * MJD doubles.
 *
 * `TimeTraits` specialisations for every `Time<S>` are provided automatically
 * by `time_base.hpp`.  A manual `CivilTime` specialisation is kept here for
 * `Period<CivilTime>` (i.e. `UTCPeriod`).
 */

#include "qtty/qtty.hpp"
#include "time.hpp"
#include <ostream>
#include <vector>

namespace tempoch {

// ============================================================================
// TimeTraits — CivilTime specialisation
// ============================================================================
// TimeTraits<Time<S>> for all scale-based types is already defined in
// time_base.hpp.  We only need the CivilTime-specific one here.

template <> struct TimeTraits<CivilTime> {
  static double to_mjd_value(const CivilTime &t) {
    return TimeScaleTraits<scales::MJD>::from_civil(t);
  }
  static CivilTime from_mjd_value(double m) { return TimeScaleTraits<scales::MJD>::to_civil(m); }
};

// ============================================================================
// Period<T>
// ============================================================================

/**
 * @brief A time period [start, end] parameterised on a time type @p T.
 *
 * Internally stores start/end as raw MJD days (matching the FFI layer) and
 * converts to/from @p T on demand via `TimeTraits<T>`.
 *
 * @tparam T A time type for which `TimeTraits<T>` is defined:
 *           `MJD` (default), `JulianDate`, or `UTC`.
 *
 * @code
 * // MJD period — must use explicit MJD wrappers to avoid JD/MJD ambiguity
 * tempoch::Period night(tempoch::MJD(60200.0), tempoch::MJD(60200.5));
 *
 * // Julian-Date period
 * auto start = tempoch::JulianDate::from_utc({2026, 1, 1});
 * auto end   = tempoch::JulianDate::from_utc({2026, 6, 1});
 * tempoch::Period jd_run(start, end);
 *
 * // UTC period
 * tempoch::Period utc_run(tempoch::UTC(2026, 1, 1), tempoch::UTC(2026, 6, 1));
 * @endcode
 */
template <typename T = MJD> class Period {
  tempoch_period_mjd_t m_inner;

  /// Private struct-based constructor used by from_c(); skips re-validation.
  explicit Period(const tempoch_period_mjd_t &inner) : m_inner(inner) {}

public:
  /**
   * @brief Construct from typed start/end values.
   *
   * Raw `double` values are intentionally not accepted to prevent the
   * JD-vs-MJD ambiguity.  Wrap the value in the appropriate type first:
   * `MJD(mjd_value)`, `JulianDate(jd_value)`, or a `UTC{…}` struct.
   *
   * @param start Inclusive start instant.
   * @param end   Inclusive end instant.
   * @throws InvalidPeriodError If @p start is later than @p end.
   */
  Period(const T &start, const T &end) {
    check_status(tempoch_period_mjd_new(TimeTraits<T>::to_mjd_value(start),
                                        TimeTraits<T>::to_mjd_value(end), &m_inner),
                 "Period::Period");
  }

  /// Construct from the C FFI struct (bypasses public validation; FFI output is
  /// trusted).
  static Period from_c(const tempoch_period_mjd_t &c) { return Period(c); }

  /// Inclusive period start as a value of type @p T.
  T start() const { return TimeTraits<T>::from_mjd_value(m_inner.start_mjd); }

  /// Inclusive period end as a value of type @p T.
  T end() const { return TimeTraits<T>::from_mjd_value(m_inner.end_mjd); }

  /**
   * @brief Duration as a qtty time quantity.
   *
   * Returns the period length converted to the requested unit.
   * The raw FFI value (in days) is wrapped as a `qtty::Day` and then
   * converted via the qtty unit-conversion layer.
   *
   * @tparam TargetType A qtty unit tag (e.g., `qtty::DayTag`) or its
   *         convenience alias (e.g., `qtty::Day`, `qtty::Second`,
   *         `qtty::Hour`). Defaults to `qtty::DayTag`.
   * @return The duration as a `qtty::Quantity` in the requested unit.
   *
   * @code
   * tempoch::Period p(tempoch::MJD(60200.0), tempoch::MJD(60201.5)); // 1.5-day
   * period auto d  = p.duration();                        // qtty::Day    → 1.5
   * d auto h  = p.duration<qtty::Hour>();            // qtty::Hour   → 36 h
   * auto s  = p.duration<qtty::SecondTag>();       // qtty::Second → 129600 s
   * @endcode
   */
  template <typename TargetType = qtty::DayTag>
  qtty::Quantity<typename qtty::ExtractTag<TargetType>::type> duration() const {
    auto qty = tempoch_period_mjd_duration_qty(m_inner);
    return qtty::Quantity<qtty::DayTag>(qty.value).template to<TargetType>();
  }

  /**
   * @brief Compute the overlapping interval with another period.
   * @param other The period to intersect with.
   * @return The overlap as a `Period<T>`.
   * @throws NoIntersectionError If the two periods do not overlap.
   */
  Period intersection(const Period &other) const {
    tempoch_period_mjd_t out;
    check_status(tempoch_period_mjd_intersection(m_inner, other.m_inner, &out),
                 "Period::intersection");
    return from_c(out);
  }

  /**
   * @brief Test whether an instant falls within this period.
   *
   * Uses a half-open [start, end) convention consistent with the FFI layer.
   *
   * @param point The instant to test.
   * @return `true` when @p point is in `[start(), end())`.
   */
  bool contains(const T &point) const noexcept {
    return tempoch_period_mjd_contains(m_inner, TimeTraits<T>::to_mjd_value(point));
  }

  /**
   * @brief Compute the union of this period and @p other.
   *
   * If the two periods overlap or touch, the result is a single merged period
   * (`result.size() == 1`).  If they are disjoint, the result contains both
   * periods in chronological order (`result.size() == 2`).
   *
   * @param other The period to unite with.
   * @return A `std::vector<Period<T>>` with one or two elements.
   * @throws InvalidPeriodError If either period is malformed.
   */
  std::vector<Period<T>> union_with(const Period<T> &other) const {
    tempoch_period_mjd_t buf[2];
    std::size_t count = 0;
    check_status(tempoch_period_mjd_union(m_inner, other.m_inner, buf, &count),
                 "Period::union_with");
    std::vector<Period<T>> result;
    result.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
      result.push_back(from_c(buf[i]));
    return result;
  }

  /**
   * @brief Compute the complement of a period list within this period.
   *
   * Returns the gaps inside `*this` that are not covered by any period in
   * @p others.  @p others must be sorted and non-overlapping.
   *
   * @param others Sorted, non-overlapping inner periods (may be empty).
   * @return The gaps as a `std::vector<Period<T>>`.
   * @throws InvalidPeriodError        If any period is malformed.
   * @throws PeriodListUnsortedError    If @p others is not sorted.
   * @throws PeriodListOverlappingError If @p others has overlapping intervals.
   */
  std::vector<Period<T>> complement_of(const std::vector<Period<T>> &others) const {
    std::vector<tempoch_period_mjd_t> raw;
    raw.reserve(others.size());
    for (const auto &p : others)
      raw.push_back(p.c_inner());

    tempoch_period_mjd_t *out_ptr = nullptr;
    std::size_t out_count = 0;
    check_status(
        tempoch_period_list_complement(m_inner, raw.data(), raw.size(), &out_ptr, &out_count),
        "Period::complement_of");

    std::vector<Period<T>> result;
    result.reserve(out_count);
    for (std::size_t i = 0; i < out_count; ++i)
      result.push_back(from_c(out_ptr[i]));
    tempoch_period_mjd_free(out_ptr, out_count);
    return result;
  }

  /// Access the underlying FFI POD value.
  const tempoch_period_mjd_t &c_inner() const { return m_inner; }
};

// ============================================================================
// C++17 deduction guides
// ============================================================================

/// Typed time values → Period<T> (covers MJD, JulianDate, UTC, …).
template <typename T> Period(T, T) -> Period<T>;

// ============================================================================
// Convenience type aliases
// ============================================================================

using MJDPeriod = Period<MJD>;       ///< Period expressed in Modified Julian Date.
using JDPeriod = Period<JulianDate>; ///< Period expressed in Julian Date.
using UTCPeriod = Period<CivilTime>; ///< Period expressed in UTC civil time.

// ============================================================================
// Period list free functions
// ============================================================================

/**
 * @brief Validate that a list of periods is sorted and non-overlapping.
 *
 * @tparam T Time type (e.g. `MJD`, `JulianDate`, `CivilTime`).
 * @param periods The list to check (may be empty).
 * @throws InvalidPeriodError        If any period has start > end.
 * @throws PeriodListUnsortedError    If the list is not sorted.
 * @throws PeriodListOverlappingError If the list has overlapping intervals.
 */
template <typename T> inline void validate_periods(const std::vector<Period<T>> &periods) {
  std::vector<tempoch_period_mjd_t> raw;
  raw.reserve(periods.size());
  for (const auto &p : periods)
    raw.push_back(p.c_inner());
  check_status(tempoch_period_list_validate(raw.data(), raw.size()), "validate_periods");
}

/**
 * @brief Intersect two sorted, non-overlapping period lists.
 *
 * @tparam T Time type.
 * @param a First sorted, non-overlapping list.
 * @param b Second sorted, non-overlapping list.
 * @return Sorted intersection of the two lists.
 * @throws InvalidPeriodError, PeriodListUnsortedError, PeriodListOverlappingError
 *         if either input list is invalid.
 */
template <typename T>
inline std::vector<Period<T>> intersect_periods(const std::vector<Period<T>> &a,
                                                const std::vector<Period<T>> &b) {
  std::vector<tempoch_period_mjd_t> ra, rb;
  ra.reserve(a.size());
  rb.reserve(b.size());
  for (const auto &p : a)
    ra.push_back(p.c_inner());
  for (const auto &p : b)
    rb.push_back(p.c_inner());

  tempoch_period_mjd_t *out_ptr = nullptr;
  std::size_t out_count = 0;
  check_status(tempoch_period_list_intersect(ra.data(), ra.size(), rb.data(), rb.size(), &out_ptr,
                                             &out_count),
               "intersect_periods");

  std::vector<Period<T>> result;
  result.reserve(out_count);
  for (std::size_t i = 0; i < out_count; ++i)
    result.push_back(Period<T>::from_c(out_ptr[i]));
  tempoch_period_mjd_free(out_ptr, out_count);
  return result;
}

/**
 * @brief Merge two period lists, combining overlapping and adjacent intervals.
 *
 * The inputs need not be sorted or non-overlapping; the output is always
 * sorted and non-overlapping.
 *
 * @tparam T Time type.
 * @param a First list.
 * @param b Second list.
 * @return Merged, normalised period list.
 * @throws InvalidPeriodError if any input period is malformed.
 */
template <typename T>
inline std::vector<Period<T>> union_periods(const std::vector<Period<T>> &a,
                                            const std::vector<Period<T>> &b) {
  std::vector<tempoch_period_mjd_t> ra, rb;
  ra.reserve(a.size());
  rb.reserve(b.size());
  for (const auto &p : a)
    ra.push_back(p.c_inner());
  for (const auto &p : b)
    rb.push_back(p.c_inner());

  tempoch_period_mjd_t *out_ptr = nullptr;
  std::size_t out_count = 0;
  check_status(
      tempoch_period_list_union(ra.data(), ra.size(), rb.data(), rb.size(), &out_ptr, &out_count),
      "union_periods");

  std::vector<Period<T>> result;
  result.reserve(out_count);
  for (std::size_t i = 0; i < out_count; ++i)
    result.push_back(Period<T>::from_c(out_ptr[i]));
  tempoch_period_mjd_free(out_ptr, out_count);
  return result;
}

/**
 * @brief Sort and merge overlapping or adjacent periods in a single list.
 *
 * @tparam T Time type.
 * @param periods The list to normalise (may be unsorted / overlapping).
 * @return Sorted, non-overlapping list.
 * @throws InvalidPeriodError if any period is malformed.
 */
template <typename T>
inline std::vector<Period<T>> normalize_periods(const std::vector<Period<T>> &periods) {
  std::vector<tempoch_period_mjd_t> raw;
  raw.reserve(periods.size());
  for (const auto &p : periods)
    raw.push_back(p.c_inner());

  tempoch_period_mjd_t *out_ptr = nullptr;
  std::size_t out_count = 0;
  check_status(tempoch_period_list_normalize(raw.data(), raw.size(), &out_ptr, &out_count),
               "normalize_periods");

  std::vector<Period<T>> result;
  result.reserve(out_count);
  for (std::size_t i = 0; i < out_count; ++i)
    result.push_back(Period<T>::from_c(out_ptr[i]));
  tempoch_period_mjd_free(out_ptr, out_count);
  return result;
}

// ============================================================================
// operator<<
// ============================================================================

/// Stream a Period as [start, end] using T's own operator<<.
template <typename T> inline std::ostream &operator<<(std::ostream &os, const Period<T> &p) {
  return os << '[' << p.start() << ", " << p.end() << ']';
}

} // namespace tempoch
