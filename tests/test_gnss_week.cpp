// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

// Tests for the GNSS week-number decomposition mirrored from tempoch.

#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

namespace {

// GPST week-0/second-0 epoch (1980-01-06T00:00:00 UTC) in J2000 seconds.
constexpr double kGpstEpochJ2000Seconds = -630'763'200.0;

} // namespace

TEST(GnssWeek, GpstEpochIsWeekZero) {
  auto t = Time<scale::GPST>::from_raw_j2000_seconds(qtty::Second(kGpstEpochJ2000Seconds));
  GnssWeek gw = to_gnss_week(t);
  EXPECT_EQ(gw.week, 0u);
  EXPECT_EQ(gw.seconds_of_week, 0u);
  EXPECT_EQ(gw.subsecond_nanos, 0u);
}

TEST(GnssWeek, RoundTripPreservesInstant) {
  // Two weeks and 3661 seconds after the GPST epoch.
  GnssWeek original{2u, 3'661u, 250'000'000u};
  auto t = from_gnss_week<scale::GPST>(original);
  GnssWeek decoded = to_gnss_week(t);
  EXPECT_EQ(decoded.week, original.week);
  EXPECT_EQ(decoded.seconds_of_week, original.seconds_of_week);
  EXPECT_NEAR(static_cast<double>(decoded.subsecond_nanos),
              static_cast<double>(original.subsecond_nanos), 1.0e3);
}

TEST(GnssWeek, QzsstAlignedWithGpst) {
  auto gp = Time<scale::GPST>::from_raw_j2000_seconds(qtty::Second(kGpstEpochJ2000Seconds));
  auto qz = Time<scale::QZSST>::from_raw_j2000_seconds(qtty::Second(kGpstEpochJ2000Seconds));
  GnssWeek g = to_gnss_week(gp);
  GnssWeek q = to_gnss_week(qz);
  EXPECT_EQ(g.week, q.week);
  EXPECT_EQ(g.seconds_of_week, q.seconds_of_week);
}
