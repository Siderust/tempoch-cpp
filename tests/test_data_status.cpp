// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

// Tests for the active time-data status API mirrored from tempoch.

#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

TEST(DataStatus, DeltaTHorizonsAreFinite) {
  TimeDataStatus status = time_data_status();
  EXPECT_TRUE(std::isfinite(status.horizons.modern_delta_t_observed_end_mjd));
  EXPECT_TRUE(std::isfinite(status.horizons.delta_t_prediction_horizon_mjd));
  // The prediction horizon must not precede the last observed ΔT MJD.
  EXPECT_GE(status.horizons.delta_t_prediction_horizon_mjd,
            status.horizons.modern_delta_t_observed_end_mjd);
}

TEST(DataStatus, SourceIsValidEnum) {
  TimeDataStatus status = time_data_status();
  EXPECT_TRUE(status.source == TimeDataSource::Bundled ||
              status.source == TimeDataSource::RuntimeCache ||
              status.source == TimeDataSource::Override);
}

TEST(DataStatus, EopHorizonsConsistent) {
  TimeDataStatus status = time_data_status();
  // EOP horizons are all-present or all-absent together.
  const bool has_start = status.horizons.eop_start_mjd.has_value();
  const bool has_obs = status.horizons.eop_observed_end_mjd.has_value();
  const bool has_end = status.horizons.eop_end_mjd.has_value();
  EXPECT_EQ(has_start, has_obs);
  EXPECT_EQ(has_obs, has_end);
  if (has_start && has_end) {
    EXPECT_LE(*status.horizons.eop_start_mjd, *status.horizons.eop_end_mjd);
  }
}
