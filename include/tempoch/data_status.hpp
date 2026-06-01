#pragma once

/**
 * @file data_status.hpp
 * @brief Active time-data status mirroring `tempoch::time_data_status`.
 *
 * Exposes the validity horizons of the currently active time-data bundle
 * (EOP and ΔT MJD horizons) together with the source the bundle was loaded
 * from.  Optional EOP horizons surface as NaN when no EOP data is loaded,
 * matching the `Option<f64>` fields on the Rust side.
 */

#include "ffi_core.hpp"

#include <cmath>
#include <optional>

namespace tempoch {

/// Origin of the currently active time-data bundle.
///
/// Mirrors `tempoch::ActiveTimeDataSource`.
enum class TimeDataSource : int {
  /// Compiled archive snapshot bundled at build time.
  Bundled = 0,
  /// Bundle loaded through the runtime fetch/cache path.
  RuntimeCache = 1,
  /// Test or caller-provided override is active.
  Override = 2,
};

/// Documented validity horizons of the active time-data bundle, in MJD (UTC).
///
/// Mirrors `tempoch::DataHorizons`: EOP horizons are absent (`std::nullopt`)
/// when no EOP data is loaded; the ΔT horizons are always present.
struct DataHorizons {
  /// First MJD covered by the EOP series, or `std::nullopt` when unloaded.
  std::optional<double> eop_start_mjd;
  /// Last observed (non-predicted) EOP MJD, or `std::nullopt` when unloaded.
  std::optional<double> eop_observed_end_mjd;
  /// Last EOP MJD including predictions, or `std::nullopt` when unloaded.
  std::optional<double> eop_end_mjd;
  /// Last MJD with observed ΔT in the archive-provided modern table.
  double modern_delta_t_observed_end_mjd;
  /// Last MJD covered by the ΔT prediction table.
  double delta_t_prediction_horizon_mjd;
};

/// Active time-data status: validity horizons plus the active bundle source.
///
/// Mirrors the numeric and provenance-source portions of
/// `tempoch::TimeDataStatus`.
struct TimeDataStatus {
  /// Validity horizons of the active bundle.
  DataHorizons horizons;
  /// Source the active bundle was loaded from.
  TimeDataSource source;
};

/// Capture the status of the currently active time-data bundle.
///
/// Mirrors `tempoch::time_data_status()`.
inline TimeDataStatus time_data_status() {
  TempochDataHorizons raw{};
  check_status(tempoch_time_data_status(&raw), "tempoch::time_data_status");

  auto opt = [](double v) -> std::optional<double> {
    return std::isnan(v) ? std::nullopt : std::optional<double>(v);
  };

  TimeDataStatus status;
  status.horizons.eop_start_mjd = opt(raw.eop_start_mjd);
  status.horizons.eop_observed_end_mjd = opt(raw.eop_observed_end_mjd);
  status.horizons.eop_end_mjd = opt(raw.eop_end_mjd);
  status.horizons.modern_delta_t_observed_end_mjd = raw.modern_delta_t_observed_end_mjd;
  status.horizons.delta_t_prediction_horizon_mjd = raw.delta_t_prediction_horizon_mjd;
  status.source = static_cast<TimeDataSource>(raw.source);
  return status;
}

} // namespace tempoch
