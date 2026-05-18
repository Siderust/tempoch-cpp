#pragma once

/**
 * @file period.hpp
 * @brief Generic time periods parameterised by a typed time representation.
 */

#include "qtty/qtty.hpp"
#include "time.hpp"
#include <ostream>
#include <vector>

namespace tempoch {

template <typename S> struct TimeTraits<Time<S>> {
  static double to_mjd_value(const Time<S> &time) {
    return time.template to<format::MJD>().value();
  }

  static Time<S> from_mjd_value(double mjd) {
    return Time<S>::from_encoded(ModifiedJulianDate<S>(mjd));
  }
};

template <typename S, typename F> struct TimeTraits<EncodedTime<S, F>> {
  static double to_mjd_value(const EncodedTime<S, F> &time) {
    if constexpr (std::is_same_v<F, format::MJD>) {
      return time.value();
    } else {
      return Time<S>::from_encoded(time).template to<format::MJD>().value();
    }
  }

  static EncodedTime<S, F> from_mjd_value(double mjd) {
    if constexpr (std::is_same_v<F, format::MJD>) {
      return EncodedTime<S, F>(mjd);
    } else {
      return ModifiedJulianDate<S>(mjd).template to<F>();
    }
  }
};

template <> struct TimeTraits<CivilTime> {
  static double to_mjd_value(const CivilTime &time) {
    return Time<scale::UTC>::from_civil(time).template to<format::MJD>().value();
  }

  static CivilTime from_mjd_value(double mjd) {
    return Time<scale::UTC>::from_encoded(ModifiedJulianDate<scale::UTC>(mjd)).to_civil();
  }
};

template <typename T = ModifiedJulianDate<scale::TT>> class Period {
  tempoch_period_mjd_t m_inner;

  explicit Period(const tempoch_period_mjd_t &inner) : m_inner(inner) {}

public:
  /// Construct a half-open interval [start, end).
  ///
  /// The period is half-open: @p start is included, @p end is excluded.
  /// @throws InvalidPeriodError if start > end.
  Period(const T &start, const T &end) {
    check_status(tempoch_period_mjd_new(TimeTraits<T>::to_mjd_value(start),
                                        TimeTraits<T>::to_mjd_value(end), &m_inner),
                 "Period::Period");
  }

  static Period from_c(const tempoch_period_mjd_t &c) { return Period(c); }

  T start() const { return TimeTraits<T>::from_mjd_value(m_inner.start_mjd); }
  T end() const { return TimeTraits<T>::from_mjd_value(m_inner.end_mjd); }

  template <typename TargetType = qtty::DayTag>
  qtty::Quantity<typename qtty::ExtractTag<TargetType>::type> duration() const {
    auto qty = tempoch_period_mjd_duration_qty(m_inner);
    return qtty::Quantity<qtty::DayTag>(qty.value).template to<TargetType>();
  }

  /// Returns the length of the period (primary name; `duration()` is a backward-compat alias).
  template <typename TargetType = qtty::DayTag>
  qtty::Quantity<typename qtty::ExtractTag<TargetType>::type> length() const {
    return duration<TargetType>();
  }

  Period intersection(const Period &other) const {
    tempoch_period_mjd_t out{};
    check_status(tempoch_period_mjd_intersection(m_inner, other.m_inner, &out),
                 "Period::intersection");
    return from_c(out);
  }

  bool contains(const T &point) const noexcept {
    return tempoch_period_mjd_contains(m_inner, TimeTraits<T>::to_mjd_value(point));
  }

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

  std::vector<Period<T>> complement_of(const std::vector<Period<T>> &others) const {
    std::vector<tempoch_period_mjd_t> raw;
    raw.reserve(others.size());
    for (const auto &p : others)
      raw.push_back(p.c_inner());
    tempoch_period_mjd_t *out = nullptr;
    std::size_t n = 0;
    check_status(tempoch_period_list_complement(m_inner, raw.data(), raw.size(), &out, &n),
                 "Period::complement_of");
    std::vector<Period<T>> result;
    result.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
      result.push_back(from_c(out[i]));
    tempoch_period_mjd_free(out, n);
    return result;
  }

  const tempoch_period_mjd_t &c_inner() const noexcept { return m_inner; }
};

template <typename T> Period(T, T) -> Period<T>;

using TTMjdPeriod = Period<ModifiedJulianDate<scale::TT>>;
using UTCPeriod = Period<CivilTime>;

namespace detail {

template <typename T>
inline std::vector<tempoch_period_mjd_t> to_raw(const std::vector<Period<T>> &periods) {
  std::vector<tempoch_period_mjd_t> raw;
  raw.reserve(periods.size());
  for (const auto &p : periods)
    raw.push_back(p.c_inner());
  return raw;
}

template <typename T>
inline std::vector<Period<T>> from_alloc(tempoch_period_mjd_t *ptr, std::size_t count) {
  std::vector<Period<T>> result;
  result.reserve(count);
  for (std::size_t i = 0; i < count; ++i)
    result.push_back(Period<T>::from_c(ptr[i]));
  tempoch_period_mjd_free(ptr, count);
  return result;
}

} // namespace detail

template <typename T> inline void validate_periods(const std::vector<Period<T>> &periods) {
  auto raw = detail::to_raw(periods);
  check_status(tempoch_period_list_validate(raw.data(), raw.size()), "validate_periods");
}

template <typename T>
inline std::vector<Period<T>> intersect_periods(const std::vector<Period<T>> &a,
                                                const std::vector<Period<T>> &b) {
  auto ra = detail::to_raw(a), rb = detail::to_raw(b);
  tempoch_period_mjd_t *out = nullptr;
  std::size_t n = 0;
  check_status(tempoch_period_list_intersect(ra.data(), ra.size(), rb.data(), rb.size(), &out, &n),
               "intersect_periods");
  return detail::from_alloc<T>(out, n);
}

template <typename T>
inline std::vector<Period<T>> union_periods(const std::vector<Period<T>> &a,
                                            const std::vector<Period<T>> &b) {
  auto ra = detail::to_raw(a), rb = detail::to_raw(b);
  tempoch_period_mjd_t *out = nullptr;
  std::size_t n = 0;
  check_status(tempoch_period_list_union(ra.data(), ra.size(), rb.data(), rb.size(), &out, &n),
               "union_periods");
  return detail::from_alloc<T>(out, n);
}

template <typename T>
inline std::vector<Period<T>> normalize_periods(const std::vector<Period<T>> &periods) {
  auto raw = detail::to_raw(periods);
  tempoch_period_mjd_t *out = nullptr;
  std::size_t n = 0;
  check_status(tempoch_period_list_normalize(raw.data(), raw.size(), &out, &n),
               "normalize_periods");
  return detail::from_alloc<T>(out, n);
}

template <typename T> inline std::ostream &operator<<(std::ostream &os, const Period<T> &period) {
  return os << '[' << period.start() << ", " << period.end() << ')';
}

} // namespace tempoch
