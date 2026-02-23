#pragma once

/**
 * @file period.hpp
 * @brief C++ wrapper for time periods, generic over any time type.
 *
 * Wraps the tempoch-ffi Period API with a value-semantic, exception-safe C++
 * class template `Period<T>`.  The underlying storage is always
 * `tempoch_period_mjd_t`; `TimeTraits<T>` handles conversion to/from the raw
 * MJD doubles.
 */

#include "time.hpp"
#include "qtty/qtty.hpp"
#include <ostream>
#include <vector>

namespace tempoch {

// ============================================================================
// TimeTraits — connect each time type to the MJD-based FFI layer
// ============================================================================

/**
 * @brief Conversion traits between a time type @p T and raw MJD doubles.
 *
 * Specialise this struct to make `Period<T>` work for a custom time type.
 * Each specialisation must provide:
 *   - `static double to_mjd_value(const T&)`
 *   - `static T      from_mjd_value(double)`
 */
template<typename T> struct TimeTraits;

template<> struct TimeTraits<MJD> {
    static double to_mjd_value(const MJD& t)  { return t.value(); }
    static MJD    from_mjd_value(double m)     { return MJD(m); }
};

template<> struct TimeTraits<JulianDate> {
    static double     to_mjd_value(const JulianDate& t) { return t.to_mjd(); }
    static JulianDate from_mjd_value(double m)          { return MJD(m).to_jd(); }
};

template<> struct TimeTraits<UTC> {
    static double to_mjd_value(const UTC& t) { return MJD::from_utc(t).value(); }
    static UTC    from_mjd_value(double m)   { return MJD(m).to_utc(); }
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
template<typename T = MJD>
class Period {
    tempoch_period_mjd_t m_inner;

    /// Private struct-based constructor used by from_c(); skips re-validation.
    explicit Period(const tempoch_period_mjd_t& inner) : m_inner(inner) {}

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
    Period(const T& start, const T& end) {
        check_status(
            tempoch_period_mjd_new(
                TimeTraits<T>::to_mjd_value(start),
                TimeTraits<T>::to_mjd_value(end),
                &m_inner),
            "Period::Period"
        );
    }

    /// Construct from the C FFI struct (bypasses public validation; FFI output is trusted).
    static Period from_c(const tempoch_period_mjd_t& c) {
        return Period(c);
    }


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
     * tempoch::Period p(tempoch::MJD(60200.0), tempoch::MJD(60201.5));  // 1.5-day period
     * auto d  = p.duration();                        // qtty::Day    → 1.5 d
     * auto h  = p.duration<qtty::Hour>();            // qtty::Hour   → 36 h
     * auto s  = p.duration<qtty::SecondTag>();       // qtty::Second → 129600 s
     * @endcode
     */
    template <typename TargetType = qtty::DayTag>
    qtty::Quantity<typename qtty::ExtractTag<TargetType>::type>
    duration() const {
        double days = tempoch_period_mjd_duration_days(m_inner);
        return qtty::Quantity<qtty::DayTag>(days).template to<TargetType>();
    }

    /**
     * @brief Compute the overlapping interval with another period.
     * @param other The period to intersect with.
     * @return The overlap as a `Period<T>`.
     * @throws NoIntersectionError If the two periods do not overlap.
     */
    Period intersection(const Period& other) const {
        tempoch_period_mjd_t out;
        check_status(
            tempoch_period_mjd_intersection(m_inner, other.m_inner, &out),
            "Period::intersection"
        );
        return from_c(out);
    }

    /// Access the underlying FFI POD value.
    const tempoch_period_mjd_t& c_inner() const { return m_inner; }
};

// ============================================================================
// C++17 deduction guides
// ============================================================================

/// Typed time values → Period<T> (covers MJD, JulianDate, UTC, …).
template<typename T>
Period(T, T) -> Period<T>;

// ============================================================================
// Convenience type aliases
// ============================================================================

using MJDPeriod = Period<MJD>;        ///< Period expressed in Modified Julian Date.
using JDPeriod  = Period<JulianDate>; ///< Period expressed in Julian Date.
using UTCPeriod = Period<UTC>;        ///< Period expressed in UTC civil time.

// ============================================================================
// operator<<
// ============================================================================

/// Stream a Period as [start, end] using T's own operator<<.
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Period<T>& p) {
    return os << '[' << p.start() << ", " << p.end() << ']';
}

} // namespace tempoch
