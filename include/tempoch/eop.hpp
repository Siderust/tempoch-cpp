#pragma once

/**
 * @file eop.hpp
 * @brief C++ wrapper for IERS Earth Orientation Parameters (EOP).
 *
 * Provides an `EopValues` aggregate with idiomatic `std::optional` fields for
 * values that may be absent in the upstream IERS data, and two free functions:
 *
 * - `eop_covers(double mjd_utc)` — check whether data exists for an epoch.
 * - `eop_at(double mjd_utc)`     — interpolate EOP values, returning
 *                                  `std::optional<EopValues>`.
 */

#include "ffi_core.hpp"
#include <cmath>
#include <optional>

namespace tempoch {

// ============================================================================
// EopValues
// ============================================================================

/**
 * @brief Interpolated IERS Earth Orientation Parameter values.
 *
 * Mirrors `tempoch_eop_values_t` with `std::optional<double>` replacing NaN
 * sentinels and `bool` replacing the `uint8_t` flag.
 *
 * All field units match the upstream IERS `finals2000A.all` file:
 * - Polar-motion components in **arcseconds**.
 * - DUT1 (UT1 − UTC) in **seconds of time**.
 * - Excess length-of-day in **milliseconds of time**.
 * - Celestial-pole offsets in **milliarcseconds**.
 */
struct EopValues {
  /// UTC MJD of the interpolation result.
  double mjd_utc;
  /// Polar motion X component (arcseconds).  `nullopt` if absent.
  std::optional<double> pm_xp_arcsec;
  /// Polar motion Y component (arcseconds).  `nullopt` if absent.
  std::optional<double> pm_yp_arcsec;
  /// DUT1 = UT1 − UTC (seconds of time).
  double ut1_minus_utc;
  /// Excess length-of-day (milliseconds of time).  `nullopt` if absent.
  std::optional<double> lod_milliseconds;
  /// ΔX celestial-pole offset (milliarcseconds).  `nullopt` if absent.
  std::optional<double> dx_milliarcsec;
  /// ΔY celestial-pole offset (milliarcseconds).  `nullopt` if absent.
  std::optional<double> dy_milliarcsec;
  /// True when both bracketing rows carry observed (`I`) data.
  bool ut1_observed;
};

// ============================================================================
// Free functions
// ============================================================================

/**
 * @brief Check whether EOP data is available for `mjd_utc`.
 *
 * @param mjd_utc UTC Modified Julian Date to query.
 * @return `true` if `eop_at(mjd_utc)` would succeed.
 */
inline bool eop_covers(double mjd_utc) noexcept {
  return tempoch_eop_covers(mjd_utc);
}

/**
 * @brief Interpolate IERS EOP values at `mjd_utc`.
 *
 * @param mjd_utc UTC Modified Julian Date of the desired epoch.
 * @return `std::optional<EopValues>` containing the result, or `std::nullopt`
 *         when the epoch is outside the compiled data range.
 *
 * @code
 * if (auto eop = tempoch::eop_at(51544.5)) {
 *     std::cout << "DUT1 = " << eop->ut1_minus_utc << " s\n";
 * }
 * @endcode
 */
inline std::optional<EopValues> eop_at(double mjd_utc) {
  tempoch_eop_values_t raw{};
  tempoch_status_t status = tempoch_eop_at(mjd_utc, &raw);

  if (status == TEMPOCH_STATUS_T_UT1_HORIZON_EXCEEDED)
    return std::nullopt;

  check_status(status, "eop_at");

  auto nan_to_opt = [](double v) -> std::optional<double> {
    return std::isnan(v) ? std::nullopt : std::optional<double>{v};
  };

  return EopValues{
      raw.mjd_utc,
      nan_to_opt(raw.pm_xp_arcsec),
      nan_to_opt(raw.pm_yp_arcsec),
      raw.ut1_minus_utc,
      nan_to_opt(raw.lod_milliseconds),
      nan_to_opt(raw.dx_milliarcsec),
      nan_to_opt(raw.dy_milliarcsec),
      raw.ut1_observed != 0,
  };
}

} // namespace tempoch
