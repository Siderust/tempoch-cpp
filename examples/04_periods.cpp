// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 04_periods.cpp
 * @example 04_periods.cpp
 * @brief Period list operations: complement, intersect, normalize, validate.
 *
 * Mirrors tempoch example 04_periods.rs.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/04_periods
 */

#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  using TTMjd = ModifiedJulianDate<scale::TT>;
  using TTMjdPeriod = Period<TTMjd>;

  auto mjd = [](double v) { return TTMjd{v}; };

  TTMjdPeriod day{mjd(61'000.0), mjd(61'001.0)};

  std::vector<TTMjdPeriod> a{
      {mjd(61'000.10), mjd(61'000.30)},
      {mjd(61'000.60), mjd(61'000.85)},
  };
  std::vector<TTMjdPeriod> b{
      {mjd(61'000.00), mjd(61'000.20)},
      {mjd(61'000.70), mjd(61'001.00)},
  };

  auto overlaps = intersect_periods(a, b);
  auto gaps = day.complement_of(a);
  auto merged = normalize_periods<TTMjd>({a[0], a[1], overlaps[0]});

  validate_periods(a); // no-throw = valid

  std::cout << "overlaps : " << overlaps.size() << "\n";
  std::cout << "gaps     : " << gaps.size() << "\n";
  std::cout << "merged   : " << merged.size() << "\n";
  std::cout << "contains 61000.5? " << std::boolalpha << a[1].contains(mjd(61'000.5)) << "\n";

  auto joined = day.union_with(TTMjdPeriod{mjd(60'999.0), mjd(61'001.5)});
  std::cout << "union    : " << joined.size() << " period(s)\n";

  return 0;
}
