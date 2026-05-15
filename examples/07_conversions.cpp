// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 07_conversions.cpp
 * @example 07_conversions.cpp
 * @brief Full conversion chain: Unix → UTC → TAI → TT → TDB / UT1 / GPS.
 *
 * Mirrors tempoch example 07_conversions.rs.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/07_conversions
 */

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  auto ctx = TimeContext::with_builtin_eop();

  // Start from a Unix timestamp (mirrors UnixTime::try_new(1_700_000_000.25)).
  UnixTime ux{1'700'000'000.25};
  auto utc = ux.to_time().to_civil();

  // Convert across continuous scales.
  auto tai = ux.to<scale::TAI>();
  auto tt = tai.to<scale::TT>();
  auto tdb = tt.to<scale::TDB>();
  auto ut1 = ux.to_with<scale::UT1>(ctx);

  // GPS bridge.
  auto gps = tai.to<format::GPS>();
  auto tai_from_gps = gps.to<scale::TAI>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "UTC          : " << utc << "\n";
  std::cout << "Unix seconds : " << std::fixed << std::setprecision(3) << ux.value() << "\n";
  std::cout << std::fixed << std::setprecision(9);
  std::cout << "TAI JD       : " << tai.to<format::JD>() << "\n";
  std::cout << "TT JD        : " << tt.to<format::JD>() << "\n";
  std::cout << "TDB JD       : " << tdb.to<format::JD>() << "\n";
  std::cout << "UT1 JD       : " << ut1.to<format::JD>() << "\n";
  std::cout << "GPS seconds  : " << gps.value() << "\n";

  // GPS → TAI round-trip residual (mirrors the assert in 07_conversions.rs).
  const double residual = std::abs((tai_from_gps - tai).value());
  std::cout << std::scientific;
  std::cout << "GPS→TAI residual: " << residual << " s\n";
  assert(residual < 1e-9);

  return 0;
}
