#pragma once

/**
 * @file tempoch.hpp
 * @brief Umbrella header for the tempoch C++ wrapper library.
 *
 * Include this single header to get the full tempoch C++ API:
 * - `tempoch::UTC` — UTC date-time breakdown
 * - `tempoch::JulianDate` — Julian Date wrapper
 * - `tempoch::MJD` — Modified Julian Date wrapper
 * - `tempoch::Period` — Time period [start, end] in MJD
 *
 * @code
 * #include <tempoch/tempoch.hpp>
 *
 * auto jd = tempoch::JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
 * auto mjd = tempoch::MJD::from_jd(jd);
 * tempoch::Period night(60200.0, 60200.5);
 * @endcode
 */

#include "ffi_core.hpp"
#include "period.hpp"
#include "time.hpp"
