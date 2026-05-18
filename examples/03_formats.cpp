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

int main() {
  using namespace tempoch;

  // J2000 epoch as canonical TT, then viewed through encodings.
  auto j2000_tt = Time<scale::TT>::from_encoded(JulianDate<scale::TT>::J2000());

  // Sample TT instant: J2000 + 123 456.789 s.
  auto sample_tt = j2000_tt + qtty::Second(123'456.789);

  // J2000 from JD scalar (matches Rust example using `j2000_jd_tt()`).
  JulianDate<scale::TT> j2000_from_jd{constants::j2000_jd_tt()};

  // Unix epoch from JD value (ABI-stable constants wrapper).
  JulianDate<scale::TT> unix_epoch_jd{constants::unix_epoch_jd()};

  // Half-day after J2000 via JD.
  JulianDate<scale::TT> half_day_jd{constants::j2000_jd_tt() + 0.5};

  // Unix epoch from MJD value.
  ModifiedJulianDate<scale::TT> unix_epoch_mjd{constants::unix_epoch_mjd()};

  // Unix time from seconds (mirrors UnixTime::try_new(Second::new(1_700_000_000.25))).
  UnixTime ux{1'700'000'000.25};
  auto utc_from_ux = Time<scale::UTC>::from_encoded(ux).to_civil();

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
