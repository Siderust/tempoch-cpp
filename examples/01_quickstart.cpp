// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 01_quickstart.cpp
 * @example 01_quickstart.cpp
 * @brief Quick start: a fixed UTC instant expressed as TT, JD, and MJD.
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

  UTC utc_now{2026, 7, 15, 22, 0, 0};
  JulianDate jd = JulianDate::from_utc(utc_now);
  TT tt = jd.to<scales::TT>();
  MJD mjd = jd.to<scales::MJD>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "UTC       : " << utc_now << "\n";
  std::cout << "TT JD     : " << tt << "\n";
  std::cout << "JD        : " << jd << "\n";
  std::cout << "MJD       : " << mjd << "\n";

  return 0;
}
