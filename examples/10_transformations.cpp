// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 10_transformations.cpp
 * @example 10_transformations.cpp
 * @brief Transform times and periods by format, by scale, or by both at once.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/10_transformations
 */

#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  auto ctx = TimeContext::with_builtin_eop();

  CivilTime civil{2026, 1, 1, 0, 0, 0};
  auto utc = Time<scale::UTC>::from_civil(civil);

  // 1. Transform only the scale: Time<UTC> -> Time<TT>.
  auto tt = utc.to<scale::TT>();

  // 2. Transform only the format: Time<UTC> -> EncodedTime<UTC, JD>.
  auto utc_jd = utc.to<format::JD>();
  auto utc_mjd = utc_jd.to<format::MJD>();

  // 3. Transform scale and format in one call:
  //    Time<UTC> -> EncodedTime<TT, MJD>.
  auto tt_mjd = utc.to<scale::TT, format::MJD>();

  // 4. Encoded values can be decoded and transformed as well.
  JulianDate<scale::UTC> jd_utc{2'460'676.5};
  auto from_jd_to_tt = jd_utc.to<scale::TT>();
  auto from_jd_to_tt_mjd = jd_utc.to<scale::TT, format::MJD>();

  // 5. UT1 uses an explicit context.
  auto ut1 = utc.to_with<scale::UT1>(ctx);
  auto ut1_mjd = utc.to_with<scale::UT1, format::MJD>(ctx);

  // 6. Periods use the same conversion shape on both endpoints.
  Period utc_window(utc, utc + qtty::Hour(12.0));
  auto tt_window = utc_window.to<scale::TT>();
  auto utc_mjd_window = utc_window.to<format::MJD>();
  auto tt_mjd_window = utc_window.to<scale::TT, format::MJD>();

  UTCPeriod civil_window(CivilTime(2026, 1, 1), CivilTime(2026, 1, 2));
  auto civil_as_tt_mjd = civil_window.to<scale::TT, format::MJD>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "Civil UTC input       : " << civil << "\n";
  std::cout << "Time<UTC> as JD       : " << utc_jd << "\n";
  std::cout << "Time<UTC> as MJD      : " << utc_mjd << "\n";
  std::cout << "Time<UTC> -> Time<TT> : " << tt.to<format::JD>() << "\n";
  std::cout << "Time<UTC> -> TT MJD   : " << tt_mjd << "\n";
  std::cout << "JD(UTC) -> Time<TT>   : " << from_jd_to_tt.to<format::JD>() << "\n";
  std::cout << "JD(UTC) -> TT MJD     : " << from_jd_to_tt_mjd << "\n";
  std::cout << "Time<UTC> -> UT1 JD   : " << ut1.to<format::JD>() << "\n";
  std::cout << "Time<UTC> -> UT1 MJD  : " << ut1_mjd << "\n";

  std::cout << "\nPeriod conversions\n";
  std::cout << "UTC window start JD   : " << utc_window.start().to<format::JD>() << "\n";
  std::cout << "TT window start JD    : " << tt_window.start().to<format::JD>() << "\n";
  std::cout << "UTC MJD window start  : " << utc_mjd_window.start() << "\n";
  std::cout << "TT MJD window start   : " << tt_mjd_window.start() << "\n";
  std::cout << "Civil -> TT MJD start : " << civil_as_tt_mjd.start() << "\n";

  return 0;
}
