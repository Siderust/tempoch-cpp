// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 01_quickstart.cpp
 * @example 01_quickstart.cpp
 * @brief Quick start: civil UTC promoted to canonical time, then encoded as
 * JD(TT), JD(UTC), and MJD(TT).
 *
 * Mirrors tempoch example 01_quickstart.rs.  chrono system-time integration
 * is not available through the C FFI, so a fixed reference date is used.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/01_quickstart
 */

#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  CivilTime civil{2026, 7, 15, 22, 0, 0};
  auto utc = Time<scale::UTC>::from_civil(civil);
  auto tt = utc.to<scale::TT>();
  auto jd_tt = tt.to<format::JD>();
  auto jd_utc = utc.to<format::JD>();
  auto mjd_tt = tt.to<format::MJD>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "Civil UTC : " << civil << "\n";
  std::cout << "UTC axis  : " << utc << "\n";
  std::cout << "TT axis   : " << tt << "\n";
  std::cout << "JD(TT)    : " << jd_tt << "\n";
  std::cout << "JD(UTC)   : " << jd_utc << "\n";
  std::cout << "MJD(TT)   : " << mjd_tt << "\n";

  return 0;
}
