#pragma once

/**
 * @file period.hpp
 * @brief C++ wrapper for time periods in MJD.
 *
 * Wraps the tempoch-ffi Period API with a value-semantic, exception-safe C++ type.
 */

#include "time.hpp"
#include <vector>

namespace tempoch {

// ============================================================================
// Period
// ============================================================================

/**
 * @brief A time period [start, end] in MJD.
 *
 * Wraps `tempoch_period_mjd_t` and provides duration, intersection,
 * and easy access to the start/end as `MJD` objects.
 *
 * @code
 * tempoch::Period night(60200.0, 60200.5);
 * double hours = night.duration_days() * 24.0;
 *
 * tempoch::Period obs(60200.2, 60200.8);
 * auto overlap = night.intersection(obs);   // [0.2, 0.5]
 * @endcode
 */
class Period {
    tempoch_period_mjd_t m_inner;

public:
    Period(double start_mjd, double end_mjd) {
        check_status(
            tempoch_period_mjd_new(start_mjd, end_mjd, &m_inner),
            "Period::Period"
        );
    }

    Period(const MJD& start, const MJD& end)
        : Period(start.value(), end.value()) {}

    /// Construct from the C struct (unchecked).
    static Period from_c(const tempoch_period_mjd_t& c) {
        Period p(0.0, 1.0); // dummy
        p.m_inner = c;
        return p;
    }

    double start_mjd() const { return m_inner.start_mjd; }
    double end_mjd()   const { return m_inner.end_mjd; }
    MJD    start()     const { return MJD(m_inner.start_mjd); }
    MJD    end()       const { return MJD(m_inner.end_mjd); }

    /// Duration in days.
    double duration_days() const {
        return tempoch_period_mjd_duration_days(m_inner);
    }

    /// Intersection with another period. Throws if no overlap.
    Period intersection(const Period& other) const {
        tempoch_period_mjd_t out;
        check_status(
            tempoch_period_mjd_intersection(m_inner, other.m_inner, &out),
            "Period::intersection"
        );
        return from_c(out);
    }

    /// Access the underlying C struct.
    const tempoch_period_mjd_t& c_inner() const { return m_inner; }
};

} // namespace tempoch
