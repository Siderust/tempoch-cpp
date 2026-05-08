#pragma once

/**
 * @file tempoch.hpp
 * @brief Umbrella header for the tempoch C++ wrapper library.
 *
 * Include this single header to get the full tempoch C++ API:
 *
 *   - `tempoch::Time<S>`       — generic time-point template (core)
 *   - `tempoch::JulianDate`    = `Time<JDScale>`
 *   - `tempoch::MJD`           = `Time<MJDScale>`
 *   - `tempoch::CivilTime`     — UTC civil date-time breakdown
 *   - `tempoch::UTC`           — alias for `CivilTime`
 *   - `tempoch::Period<T>`     — time period [start, end] with list operations
 *   - `tempoch::EopValues`     — IERS Earth Orientation Parameter values
 *   - `tempoch::eop_at()`      — interpolate EOP at a UTC MJD
 *   - `tempoch::eop_covers()`  — check EOP data availability
 *   - `tempoch::constants::`   — named astronomical constants
 *
 * @code
 * #include <tempoch/tempoch.hpp>
 *
 * auto jd  = tempoch::JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
 * auto mjd = jd.to<tempoch::MJDScale>();   // cross-scale conversion
 *
 * if (auto eop = tempoch::eop_at(mjd.value()))
 *     std::cout << "DUT1 = " << eop->ut1_minus_utc << " s\n";
 *
 * std::cout << "J2000 = " << tempoch::constants::j2000_jd_tt() << " JD\n";
 * @endcode
 */

#include "constants.hpp"
#include "eop.hpp"
#include "ffi_core.hpp"
#include "period.hpp"
#include "scales.hpp"
#include "time.hpp"
#include "time_base.hpp"
