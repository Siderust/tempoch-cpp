#pragma once

/**
 * @file tempoch.hpp
 * @brief Umbrella header for the tempoch C++ wrapper library.
 *
 * Include this single header to get the full tempoch C++ API:
 *
 *   - `tempoch::Time<S>`         — split-storage instant on scale `S`
 *   - `tempoch::JulianDate<S>`   — JD encoding on scale `S`
 *   - `tempoch::ModifiedJulianDate<S>`
 *   - `tempoch::CivilTime`       — civil UTC calendar label
 *   - `tempoch::TimeContext`     — explicit UT1 / historical UTC context
 *   - `tempoch::Period<T>`       — time period [start, end] with list operations
 *   - `tempoch::EopValues`     — IERS Earth Orientation Parameter values
 *   - `tempoch::eop_at()`      — interpolate EOP at a UTC MJD
 *   - `tempoch::eop_covers()`  — check EOP data availability
 *   - `tempoch::constants::`   — named astronomical constants
 *
 * @code
 * #include <tempoch/tempoch.hpp>
 *
 * auto utc = tempoch::Time<tempoch::scale::UTC>::from_civil({2026, 7, 15, 22, 0, 0});
 * auto tt  = utc.to<tempoch::scale::TT>();
 * auto jd  = tt.to<tempoch::format::JD>();
 * auto mjd = tt.to<tempoch::format::MJD>();
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
