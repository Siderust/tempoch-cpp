// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 03_formats.cpp
 * @example 03_formats.cpp
 * @brief Constructor and accessor showcase for all Time<S> specialisations.
 *
 * Mirrors tempoch example 03_formats.rs.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/03_formats
 */

#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

// Well-known fixed values (from tempoch-core::constats).
static constexpr double UNIX_EPOCH_JD = 2'440'587.5; // 1970-01-01 JD
static constexpr double UNIX_EPOCH_MJD = 40'587.0;   // 1970-01-01 MJD

int main() {
  using namespace tempoch;

  // J2000 epoch as canonical TT, then viewed through encodings.
  auto j2000_tt = JulianDate<scale::TT>::J2000().to_time();

  // Sample TT instant: J2000 + 123 456.789 s.
  auto sample_tt = j2000_tt + qtty::Second(123'456.789);

  // J2000 from JulianDate constructor (mirrors JulianDate::<TT>::try_new(J2000_JD_TT)).
  JulianDate<scale::TT> j2000_from_jd{constants::j2000_jd_tt()};

  // Unix epoch from JD value.
  JulianDate<scale::TT> unix_epoch_jd{UNIX_EPOCH_JD};

  // Half-day after J2000 via JD.
  JulianDate<scale::TT> half_day_jd{constants::j2000_jd_tt() + 0.5};

  // Unix epoch from MJD value.
  ModifiedJulianDate<scale::TT> unix_epoch_mjd{UNIX_EPOCH_MJD};

  // Unix time from seconds (mirrors UnixTime::try_new(Second::new(1_700_000_000.25))).
  UnixTime ux{1'700'000'000.25};
  auto utc_from_ux = ux.to_time().to_civil();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "J2000 TT JD       : " << j2000_tt.to<format::JD>() << "\n";
  std::cout << "Sample TT JD      : " << sample_tt.to<format::JD>() << "\n";
  std::cout << "Sample TT MJD     : " << sample_tt.to<format::MJD>() << "\n";
  std::cout << "J2000 from JD     : " << j2000_from_jd << "\n";
  std::cout << "Unix epoch JD(TT) : " << unix_epoch_jd << "\n";
  std::cout << "Half-day JD(TT)   : " << half_day_jd << "\n";
  std::cout << "Unix epoch MJD(TT): " << unix_epoch_mjd << "\n";
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "UTC from Unix     : " << utc_from_ux << "\n";

  return 0;
}
