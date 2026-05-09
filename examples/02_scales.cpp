// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 02_scales.cpp
 * @example 02_scales.cpp
 * @brief All time-scale conversions from the J2000.0 TT epoch.
 *
 * Mirrors tempoch example 02_scales.rs.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/02_scales
 */

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  JulianDate jd{constants::j2000_jd_tt()};
  TT tt = jd.to<scales::TT>();
  TAI tai = tt.to<scales::TAI>();
  UTC utc = tt.to_utc();
  UT ut1 = tt.to<scales::UT>();
  TDB tdb = tt.to<scales::TDB>();
  TCG tcg = tt.to<scales::TCG>();
  TCB tcb = tt.to<scales::TCB>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "TT  JD  : " << tt << "\n";
  std::cout << "TAI JD  : " << tai << "\n";
  std::cout << "UT1 JD  : " << ut1 << "\n";
  std::cout << "TDB JD  : " << tdb << "\n";
  std::cout << "TCG JD  : " << tcg << "\n";
  std::cout << "TCB JD  : " << tcb << "\n";
  std::cout << "UTC     : " << utc << "\n";

  constexpr double SPD = 86'400.0;
  const double tt_tai = (tt.value() - tai.value()) * SPD;
  const double tt_ut1 = (tt.value() - ut1.value()) * SPD;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "TT-TAI  : " << tt_tai << " s\n";
  std::cout << "TT-UT1  : " << tt_ut1 << " s\n";

  assert(std::abs(tt_tai - 32.184) < 1e-9);

  return 0;
}
