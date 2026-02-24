#pragma once

/**
 * @file tempoch.hpp
 * @brief Umbrella header for the tempoch C++ wrapper library.
 *
 * Include this single header to get the full tempoch C++ API:
 *
 *   - `tempoch::Time<S>` — generic time-point template (core)
 *   - `tempoch::JulianDate` = `Time<JDScale>`
 *   - `tempoch::MJD`        = `Time<MJDScale>`
 *   - `tempoch::CivilTime`  — UTC civil date-time breakdown
 *   - `tempoch::UTC`        — alias for `CivilTime`
 *   - `tempoch::Period<T>`  — time period [start, end]
 *
 * @code
 * #include <tempoch/tempoch.hpp>
 *
 * auto jd  = tempoch::JulianDate::from_utc({2026, 7, 15, 22, 0, 0});
 * auto mjd = jd.to<tempoch::MJDScale>();   // cross-scale conversion
 * @endcode
 */

#include "ffi_core.hpp"
#include "scales.hpp"
#include "time_base.hpp"
#include "time.hpp"
#include "period.hpp"
