// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 09_gnss_week.cpp
 * @example 09_gnss_week.cpp
 * @brief GNSS week-number decomposition (GnssWeek) mirroring tempoch.
 *
 * Decomposes a GNSS-scale instant into `(week, seconds_of_week,
 * subsecond_nanos)` since the constellation's defined epoch, and rebuilds the
 * instant from that decomposition. Defined for GPST, GST, BDT and QZSST.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/09_gnss_week
 */

#include <cassert>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  // GPST week-0/second-0 epoch (1980-01-06 UTC) expressed in J2000 seconds.
  constexpr double kGpstEpochJ2000Seconds = -630'763'200.0;

  auto epoch = Time<scale::GPST>::from_raw_j2000_seconds(qtty::Second(kGpstEpochJ2000Seconds));
  GnssWeek gw0 = to_gnss_week(epoch);
  std::cout << "GPST epoch -> week=" << gw0.week << " sow=" << gw0.seconds_of_week
            << " ns=" << gw0.subsecond_nanos << '\n';
  assert(gw0.week == 0u && gw0.seconds_of_week == 0u && gw0.subsecond_nanos == 0u);

  // Build an instant two weeks + 1 hour past the epoch, then round-trip it.
  GnssWeek target{2u, 3'600u, 0u};
  auto t = from_gnss_week<scale::GPST>(target);
  GnssWeek back = to_gnss_week(t);
  std::cout << "round-trip -> week=" << back.week << " sow=" << back.seconds_of_week << '\n';
  assert(back.week == target.week);
  assert(back.seconds_of_week == target.seconds_of_week);

  // QZSST shares the GPST week numbering.
  auto qz = Time<scale::QZSST>::from_raw_j2000_seconds(qtty::Second(kGpstEpochJ2000Seconds));
  GnssWeek qzw = to_gnss_week(qz);
  std::cout << "QZSST epoch -> week=" << qzw.week << " sow=" << qzw.seconds_of_week << '\n';
  assert(qzw.week == gw0.week && qzw.seconds_of_week == gw0.seconds_of_week);

  std::cout << "GNSS week-number decomposition OK\n";
  return 0;
}
