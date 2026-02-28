#pragma once

/**
 * @file civil_time.hpp
 * @brief UTC date-time breakdown struct.
 *
 * Forward-declared in scales.hpp and fully defined here to avoid circular
 * dependencies.
 */

#include "ffi_core.hpp"
#include <iomanip>
#include <ostream>

namespace tempoch {

/**
 * @brief UTC date-time breakdown.
 *
 * A simple value type mirroring the C `tempoch_utc_t` struct.
 *
 * @code
 * tempoch::CivilTime noon(2000, 1, 1, 12, 0, 0);
 * auto jd = tempoch::JulianDate::from_utc(noon);
 * @endcode
 */
struct CivilTime {
  int32_t year;        ///< Gregorian year (astronomical year numbering).
  uint8_t month;       ///< Month [1, 12].
  uint8_t day;         ///< Day of month [1, 31].
  uint8_t hour;        ///< Hour [0, 23].
  uint8_t minute;      ///< Minute [0, 59].
  uint8_t second;      ///< Second [0, 60] (leap-second aware).
  uint32_t nanosecond; ///< Nanosecond [0, 999 999 999].

  /// Default constructor: J2000 epoch noon-like civil representation.
  CivilTime()
      : year(2000), month(1), day(1), hour(12), minute(0), second(0),
        nanosecond(0) {}

  /**
   * @brief Construct from civil UTC components.
   */
  CivilTime(int32_t y, uint8_t mo, uint8_t d, uint8_t h = 0, uint8_t mi = 0,
            uint8_t s = 0, uint32_t ns = 0)
      : year(y), month(mo), day(d), hour(h), minute(mi), second(s),
        nanosecond(ns) {}

  /// Convert to the C FFI struct.
  tempoch_utc_t to_c() const {
    return {year, month, day, hour, minute, second, nanosecond};
  }

  /// Create from the C FFI struct.
  static CivilTime from_c(const tempoch_utc_t &c) {
    return CivilTime(c.year, c.month, c.day, c.hour, c.minute, c.second,
                     c.nanosecond);
  }
};

/// Stream CivilTime as YYYY-MM-DD HH:MM:SS[.nnnnnnnnn].
inline std::ostream &operator<<(std::ostream &os, const CivilTime &u) {
  const char prev = os.fill();
  os << u.year << '-' << std::setfill('0') << std::setw(2)
     << static_cast<int>(u.month) << '-' << std::setw(2)
     << static_cast<int>(u.day) << ' ' << std::setw(2)
     << static_cast<int>(u.hour) << ':' << std::setw(2)
     << static_cast<int>(u.minute) << ':' << std::setw(2)
     << static_cast<int>(u.second);
  if (u.nanosecond != 0)
    os << '.' << std::setw(9) << u.nanosecond;
  os.fill(prev);
  return os;
}

} // namespace tempoch
