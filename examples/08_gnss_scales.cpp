// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 08_gnss_scales.cpp
 * @example 08_gnss_scales.cpp
 * @brief GNSS and SPICE-compatibility time scales (GPST, GST, QZSST, BDT, ET).
 *
 * These scales were added to mirror the full tempoch scale set. GPST, GST and
 * QZSST share the nominal `TAI - 19 s` offset; BDT is `TAI - 33 s`; ET is a
 * SPICE-compatibility marker that tracks TDB.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/08_gnss_scales
 */

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  auto tai = Time<scale::TAI>::from_split_seconds(qtty::Second(1.0e9), qtty::Second(0.0));

  const double tai_s = tai.to<format::J2000s>().value();
  const double gpst_s = tai.to<scale::GPST>().to<format::J2000s>().value();
  const double gst_s = tai.to<scale::GST>().to<format::J2000s>().value();
  const double qzsst_s = tai.to<scale::QZSST>().to<format::J2000s>().value();
  const double bdt_s = tai.to<scale::BDT>().to<format::J2000s>().value();
  const double et_s = tai.to<scale::ET>().to<format::J2000s>().value();
  const double tdb_s = tai.to<scale::TDB>().to<format::J2000s>().value();

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "TAI - GPST  : " << (tai_s - gpst_s) << " s\n";
  std::cout << "TAI - GST   : " << (tai_s - gst_s) << " s\n";
  std::cout << "TAI - QZSST : " << (tai_s - qzsst_s) << " s\n";
  std::cout << "TAI - BDT   : " << (tai_s - bdt_s) << " s\n";
  std::cout << "GPST - BDT  : " << (gpst_s - bdt_s) << " s\n";
  std::cout << "ET - TDB    : " << (et_s - tdb_s) << " s\n";

  assert(std::abs((tai_s - gpst_s) - 19.0) < 1e-6);
  assert(std::abs((tai_s - bdt_s) - 33.0) < 1e-6);
  assert(std::abs((gpst_s - bdt_s) - 14.0) < 1e-6);
  assert(std::abs(et_s - tdb_s) < 1e-9);

  return 0;
}
