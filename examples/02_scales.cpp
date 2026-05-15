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

  auto ctx = TimeContext::with_builtin_eop();

  auto jd_tt = JulianDate<scale::TT>::J2000();
  auto tt = jd_tt.to_time();
  auto tai = tt.to<scale::TAI>();
  auto utc = tt.to<scale::UTC>().to_civil();
  auto ut1 = tt.to_with<scale::UT1>(ctx);
  auto tdb = tt.to<scale::TDB>();
  auto tcg = tt.to<scale::TCG>();
  auto tcb = tt.to<scale::TCB>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "TT  JD  : " << tt.to<format::JD>() << "\n";
  std::cout << "TAI JD  : " << tai.to<format::JD>() << "\n";
  std::cout << "UT1 JD  : " << ut1.to<format::JD>() << "\n";
  std::cout << "TDB JD  : " << tdb.to<format::JD>() << "\n";
  std::cout << "TCG JD  : " << tcg.to<format::JD>() << "\n";
  std::cout << "TCB JD  : " << tcb.to<format::JD>() << "\n";
  std::cout << "UTC     : " << utc << "\n";

  constexpr double SPD = 86'400.0;
  const double tt_tai = (tt.to<format::JD>().value() - tai.to<format::JD>().value()) * SPD;
  const double tt_ut1 = (tt.to<format::JD>().value() - ut1.to<format::JD>().value()) * SPD;

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "TT-TAI  : " << tt_tai << " s\n";
  std::cout << "TT-UT1  : " << tt_ut1 << " s\n";

  assert(std::abs(tt_tai - 32.184) < 1e-9);

  return 0;
}
