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

  // Start from a Unix timestamp (mirrors UnixTime::try_new(1_700_000_000.25)).
  UnixTime ux{1'700'000'000.25};
  UTC utc = ux.to_utc();

  // Convert across continuous scales.
  TAI tai = ux.to<TAIScale>();
  TT tt = tai.to<TTScale>();
  TDB tdb = tt.to<TDBScale>();
  UT ut1 = ux.to<UTScale>();

  // GPS bridge.
  GPS gps = tai.to<GPSScale>();
  TAI tai_from_gps = gps.to<TAIScale>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "UTC          : " << utc << "\n";
  std::cout << "Unix seconds : " << std::fixed << std::setprecision(3)
            << ux.value() << "\n";
  std::cout << std::fixed << std::setprecision(9);
  std::cout << "TAI JD       : " << tai.value() << "\n";
  std::cout << "TT JD        : " << tt.value() << "\n";
  std::cout << "TDB JD       : " << tdb.value() << "\n";
  std::cout << "UT1 JD       : " << ut1.value() << "\n";
  std::cout << "GPS days     : " << gps.value() << "\n"; // days since GPS epoch

  // GPS → TAI round-trip residual (mirrors the assert in 07_conversions.rs).
  const double residual =
      std::abs(tai_from_gps.value() - tai.value()) * 86'400.0;
  std::cout << std::scientific;
  std::cout << "GPS→TAI residual: " << residual << " s\n";
  assert(residual < 1e-9);

  return 0;
}
