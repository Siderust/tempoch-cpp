// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

// Tests for the named constants and ΔT helpers newly mirrored from tempoch.

#include <cmath>

#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

TEST(Constants, NewNamedConstants) {
  EXPECT_DOUBLE_EQ(constants::tt_minus_tai_seconds(), 32.184);
  EXPECT_DOUBLE_EQ(constants::nanos_per_second(), 1.0e9);
  EXPECT_DOUBLE_EQ(constants::iau_time_epoch_t0_jd(), 2'443'144.500'372'5);
  EXPECT_DOUBLE_EQ(constants::tdb_tt_model_high_accuracy_start_jd(), 2'305'447.5);
  EXPECT_DOUBLE_EQ(constants::tdb_tt_model_high_accuracy_end_jd(), 2'524'598.5);
}

TEST(Constants, DeltaTAtJ2000) {
  // ΔT near J2000.0 is approximately 63.8 s.
  double dt = delta_t_seconds(constants::j2000_jd_tt());
  ASSERT_TRUE(std::isfinite(dt));
  EXPECT_NEAR(dt, 63.83, 1.0);
}

TEST(Constants, DeltaTExtrapolatedIsFinite) {
  // Far-future epoch: tabulated model is exceeded, extrapolation stays finite.
  double dt = delta_t_seconds_extrapolated(3'000'000.0);
  EXPECT_TRUE(std::isfinite(dt));
}
