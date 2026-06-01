#pragma once

/**
 * @file constants.hpp
 * @brief Named astronomical and time-system constants from tempoch.
 *
 * Each constant is a thin `constexpr` or `inline` wrapper that delegates to
 * the corresponding tempoch-ffi function.  Where the FFI function accesses
 * data tables built at compile-time (e.g. EOP start/end MJDs), the wrapper
 * intentionally remains a function rather than a `constexpr` to avoid ABI
 * coupling.
 */

#include "ffi_core.hpp"

namespace tempoch {
namespace constants {

/// J2000.0 epoch as a Julian Date in TT (2 451 545.0).
inline double j2000_jd_tt() noexcept { return tempoch_const_j2000_jd_tt(); }

/// Unix epoch as Julian Date on the UTC axis (`1970-01-01` midnight UTC).
inline double unix_epoch_jd() noexcept { return tempoch_const_unix_epoch_jd(); }

/// Unix epoch as Modified Julian Day on the UTC axis (`40 587.0`).
inline double unix_epoch_mjd() noexcept { return tempoch_const_unix_epoch_mjd(); }

/// Length of a Julian year in days (365.25 days).
inline double julian_year_days() noexcept { return tempoch_const_julian_year_days(); }

/// UTC MJD from which the UTC time scale is defined (1961-01-01, MJD 37300).
inline double utc_defined_from_mjd() noexcept { return tempoch_const_utc_defined_from_mjd(); }

/// GPS epoch as a Julian Date in UTC (1980-01-06T00:00:00 UTC, JD 2 444 244.5).
inline double gps_epoch_jd_utc() noexcept { return tempoch_const_gps_epoch_jd_utc(); }

/// GPS epoch as a Julian Date in TAI.
inline double gps_epoch_jd_tai() noexcept { return tempoch_const_gps_epoch_jd_tai(); }

/// TAI − UTC offset at the GPS epoch in seconds (19 s).
inline double gps_epoch_tai_minus_utc_seconds() noexcept {
  return tempoch_const_gps_epoch_tai_minus_utc_seconds();
}

/// MJD beyond which ΔT predictions are no longer reliable (UTC).
inline double delta_t_prediction_horizon_mjd() noexcept {
  return tempoch_const_delta_t_prediction_horizon_mjd();
}

/// First MJD covered by the compiled EOP data (UTC).
inline double eop_start_mjd() noexcept { return tempoch_const_eop_start_mjd(); }

/// Last MJD covered by the compiled EOP data (UTC).
inline double eop_end_mjd() noexcept { return tempoch_const_eop_end_mjd(); }

/// Last MJD for which compiled EOP data is observed (rather than predicted, UTC).
inline double eop_observed_end_mjd() noexcept { return tempoch_const_eop_observed_end_mjd(); }

/// Last MJD for which modern ΔT observed values are available (UTC).
inline double modern_delta_t_observed_end_mjd() noexcept {
  return tempoch_const_modern_delta_t_observed_end_mjd();
}

/// Constant TT − TAI offset in seconds (32.184 s).
inline double tt_minus_tai_seconds() noexcept { return tempoch_const_tt_minus_tai_seconds(); }

/// Number of nanoseconds in one SI second (1e9).
inline double nanos_per_second() noexcept { return tempoch_const_nanos_per_second(); }

/// IAU time-scale epoch T0 as a Julian Date on the TT axis (1977-01-01 TAI).
inline double iau_time_epoch_t0_jd() noexcept { return tempoch_const_iau_time_epoch_t0_jd(); }

/// First JD(TT) of the high-accuracy TDB−TT model validity window.
inline double tdb_tt_model_high_accuracy_start_jd() noexcept {
  return tempoch_const_tdb_tt_model_high_accuracy_start_jd();
}

/// Last JD(TT) of the high-accuracy TDB−TT model validity window.
inline double tdb_tt_model_high_accuracy_end_jd() noexcept {
  return tempoch_const_tdb_tt_model_high_accuracy_end_jd();
}

} // namespace constants

/// ΔT = TT − UT1 in seconds for a UT1 Julian Day, using the compiled USNO
/// model. Returns NaN when the requested epoch is outside the model domain.
inline double delta_t_seconds(double jd_ut1) noexcept { return tempoch_delta_t_seconds(jd_ut1); }

/// ΔT = TT − UT1 in seconds for a UT1 Julian Day, extrapolating beyond the
/// tabulated range with the long-term parabola (always finite).
inline double delta_t_seconds_extrapolated(double jd_ut1) noexcept {
  return tempoch_delta_t_seconds_extrapolated(jd_ut1);
}

} // namespace tempoch
