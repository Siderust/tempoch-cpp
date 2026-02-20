#pragma once

/**
 * @file time.hpp
 * @brief C++ wrappers for Julian Date, Modified Julian Date, and UTC.
 *
 * Wraps the tempoch-ffi C layer with value-semantic, exception-safe C++ types.
 */

#include "ffi_core.hpp"

namespace tempoch {

// ============================================================================
// UTC
// ============================================================================

/**
 * @brief UTC date-time breakdown.
 *
 * A simple value type mirroring the C `tempoch_utc_t` struct.
 *
 * @code
 * tempoch::UTC noon(2000, 1, 1, 12, 0, 0);
 * auto jd = tempoch::JulianDate::from_utc(noon);
 * @endcode
 */
struct UTC {
    /// Gregorian year (astronomical year numbering).
    int32_t  year;
    /// Month in range [1, 12].
    uint8_t  month;
    /// Day of month in range [1, 31].
    uint8_t  day;
    /// Hour in range [0, 23].
    uint8_t  hour;
    /// Minute in range [0, 59].
    uint8_t  minute;
    /// Second in range [0, 60], leap second aware.
    uint8_t  second;
    /// Nanosecond component in range [0, 999,999,999].
    uint32_t nanosecond;

    /// Default constructor: J2000 epoch noon-like civil representation.
    UTC() : year(2000), month(1), day(1), hour(12), minute(0), second(0), nanosecond(0) {}

    /**
     * @brief Construct from civil UTC components.
     * @param y Year.
     * @param mo Month [1, 12].
     * @param d Day [1, 31].
     * @param h Hour [0, 23].
     * @param mi Minute [0, 59].
     * @param s Second [0, 60].
     * @param ns Nanoseconds [0, 999,999,999].
     */
    UTC(int32_t y, uint8_t mo, uint8_t d,
        uint8_t h = 0, uint8_t mi = 0, uint8_t s = 0, uint32_t ns = 0)
        : year(y), month(mo), day(d), hour(h), minute(mi), second(s), nanosecond(ns) {}

    /// Convert to the C FFI struct.
    tempoch_utc_t to_c() const {
        return {year, month, day, hour, minute, second, nanosecond};
    }

    /// Create from the C FFI struct.
    static UTC from_c(const tempoch_utc_t& c) {
        return UTC(c.year, c.month, c.day, c.hour, c.minute, c.second, c.nanosecond);
    }
};

// ============================================================================
// JulianDate
// ============================================================================

/**
 * @brief Julian Date wrapper (value type).
 *
 * Wraps a raw `double` Julian Date value and provides conversions to/from UTC,
 * MJD, and arithmetic operations.
 *
 * @code
 * auto jd = tempoch::JulianDate::J2000();           // 2451545.0
 * auto utc = jd.to_utc();
 * auto jd2 = jd + 365.25;                           // add one Julian year
 * double centuries = jd.julian_centuries();           // since J2000
 * @endcode
 */
class JulianDate {
    double m_value;

public:
    constexpr explicit JulianDate(double v) : m_value(v) {}

    /// J2000.0 epoch (2451545.0).
    static JulianDate J2000() { return JulianDate(tempoch_jd_j2000()); }

    /// Create from a UTC date-time.
    static JulianDate from_utc(const UTC& utc) {
        double jd;
        auto c = utc.to_c();
        check_status(tempoch_jd_from_utc(c, &jd), "JulianDate::from_utc");
        return JulianDate(jd);
    }

    /// Raw value.
    constexpr double value() const { return m_value; }

    /// Convert to MJD.
    double to_mjd() const { return tempoch_jd_to_mjd(m_value); }

    /// Convert to UTC.
    UTC to_utc() const {
        tempoch_utc_t out;
        check_status(tempoch_jd_to_utc(m_value, &out), "JulianDate::to_utc");
        return UTC::from_c(out);
    }

    /// Difference in days (this – other).
    double operator-(const JulianDate& other) const {
        return tempoch_jd_difference(m_value, other.m_value);
    }

    /// Add days.
    JulianDate operator+(double days) const {
        return JulianDate(tempoch_jd_add_days(m_value, days));
    }

    /// Julian centuries since J2000.
    double julian_centuries() const {
        return tempoch_jd_julian_centuries(m_value);
    }

    bool operator==(const JulianDate& o) const { return m_value == o.m_value; }
    bool operator!=(const JulianDate& o) const { return m_value != o.m_value; }
    bool operator< (const JulianDate& o) const { return m_value <  o.m_value; }
    bool operator<=(const JulianDate& o) const { return m_value <= o.m_value; }
    bool operator> (const JulianDate& o) const { return m_value >  o.m_value; }
    bool operator>=(const JulianDate& o) const { return m_value >= o.m_value; }
};

// ============================================================================
// MJD (Modified Julian Date)
// ============================================================================

/**
 * @brief Modified Julian Date wrapper (value type).
 *
 * Wraps a raw `double` MJD value and provides conversions to/from UTC, JD,
 * and arithmetic operations.
 *
 * @code
 * auto mjd = tempoch::MJD::from_utc({2026, 7, 15});
 * auto jd  = mjd.to_jd();
 * auto mjd2 = mjd + 1.0;   // next day
 * @endcode
 */
class MJD {
    double m_value;

public:
    constexpr MJD() : m_value(0.0) {}
    constexpr explicit MJD(double v) : m_value(v) {}

    /// Create from a UTC date-time.
    static MJD from_utc(const UTC& utc) {
        double mjd;
        auto c = utc.to_c();
        check_status(tempoch_mjd_from_utc(c, &mjd), "MJD::from_utc");
        return MJD(mjd);
    }

    /// Create from a Julian Date.
    static MJD from_jd(const JulianDate& jd) {
        return MJD(tempoch_jd_to_mjd(jd.value()));
    }

    /// Raw value.
    constexpr double value() const { return m_value; }

    /// Convert to JD.
    JulianDate to_jd() const { return JulianDate(tempoch_mjd_to_jd(m_value)); }

    /// Convert to UTC.
    UTC to_utc() const {
        tempoch_utc_t out;
        check_status(tempoch_mjd_to_utc(m_value, &out), "MJD::to_utc");
        return UTC::from_c(out);
    }

    /// Difference in days (this – other).
    double operator-(const MJD& other) const {
        return tempoch_mjd_difference(m_value, other.m_value);
    }

    /// Add days.
    MJD operator+(double days) const {
        return MJD(tempoch_mjd_add_days(m_value, days));
    }

    bool operator==(const MJD& o) const { return m_value == o.m_value; }
    bool operator!=(const MJD& o) const { return m_value != o.m_value; }
    bool operator< (const MJD& o) const { return m_value <  o.m_value; }
    bool operator<=(const MJD& o) const { return m_value <= o.m_value; }
    bool operator> (const MJD& o) const { return m_value >  o.m_value; }
    bool operator>=(const MJD& o) const { return m_value >= o.m_value; }
};

} // namespace tempoch
