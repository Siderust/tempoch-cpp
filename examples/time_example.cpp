/**
 * @file time_example.cpp
 * @example time_example.cpp
 * @brief Demonstrates the tempoch C++ API.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/time_example
 */

#include <iomanip>
#include <iostream>
#include <qtty/qtty.hpp>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  // ---------------------------------------------------------------
  // UTC → JulianDate → MJD round-trip
  // ---------------------------------------------------------------
  UTC utc(2026, 7, 15, 22, 0, 0);
  std::cout << "UTC:  " << utc << "\n";

  auto jd = JulianDate::from_utc(utc);
  std::cout << "JD:   " << std::fixed << std::setprecision(6) << jd << "\n";

  auto mjd = MJD::from_jd(jd);
  std::cout << "MJD:  " << std::fixed << std::setprecision(6) << mjd << "\n";

  auto utc2 = mjd.to_utc();
  std::cout << "Back: " << utc2 << "\n\n";

  // ---------------------------------------------------------------
  // J2000 epoch and Julian centuries
  // ---------------------------------------------------------------
  auto j2000 = JulianDate::J2000();
  std::cout << "J2000.0:  " << j2000 << "\n";
  std::cout << "Centuries since J2000: " << jd.julian_centuries() << "\n\n";

  // ---------------------------------------------------------------
  // Period intersection (MJD — explicit MJD wrappers required)
  // ---------------------------------------------------------------
  Period night(MJD(60200.0), MJD(60200.5));
  Period obs(MJD(60200.2), MJD(60200.8));
  auto overlap = night.intersection(obs);
  std::cout << "Night:   " << night << "\n";
  std::cout << "Obs:     " << obs << "\n";
  std::cout << "Overlap: " << overlap << "\n";
  std::cout << "Overlap duration: " << overlap.duration<qtty::Hour>() << "\n\n";

  // ---------------------------------------------------------------
  // Period<UTC> — start/end expressed directly as civil UTC
  // ---------------------------------------------------------------
  Period utc_semester(UTC(2026, 1, 1), UTC(2026, 7, 1));
  std::cout << "Semester (UTC): " << utc_semester << "\n";
  std::cout << "  duration: " << utc_semester.duration() << "\n\n";

  // ---------------------------------------------------------------
  // Period<JulianDate> — start/end as Julian Dates
  // ---------------------------------------------------------------
  auto jd_start = JulianDate::from_utc(UTC(2026, 1, 1));
  auto jd_end = JulianDate::from_utc(UTC(2026, 7, 1));
  Period jd_semester(jd_start, jd_end);
  std::cout << "Semester (JD):  " << jd_semester << "\n";
  std::cout << "  duration: " << jd_semester.duration() << "\n";

  return 0;
}
