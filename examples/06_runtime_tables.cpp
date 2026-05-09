// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file 06_runtime_tables.cpp
 * @example 06_runtime_tables.cpp
 * @brief EOP table access, UT1 conversion, and astronomical constants.
 *
 * Mirrors tempoch example 06_runtime_tables.rs.  The EOP data is always
 * compiled in (no explicit TimeContext is required).
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/06_runtime_tables
 */

#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;
  using namespace tempoch::constants;

  // ── TT → UT1 (mirrors probe_tt → probe_ut1 in 06_runtime_tables.rs) ────
  // JD TT 2 460 000.25 ≈ 2023-05-15T18:00:00 TT
  JulianDate probe_tt{2'460'000.25};
  UT probe_ut1 = probe_tt.to<scales::UT>();

  std::cout << std::fixed << std::setprecision(9);
  std::cout << "probe TT JD  : " << probe_tt << "\n";
  std::cout << "probe UT1 JD : " << probe_ut1 << "\n";

  // ── Unix timestamp roundtrip ─────────────────────────────────────────────
  UnixTime ux{1'700'000'000.0};
  UTC utc_from_ux = ux.to_utc();
  UnixTime back = UnixTime::from_utc(utc_from_ux);

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Unix roundtrip: " << back << "\n\n";

  // ── EOP data range and point query ───────────────────────────────────────
  std::cout << std::fixed << std::setprecision(1);
  std::cout << "EOP range    : [" << eop_start_mjd() << ", " << eop_end_mjd() << "] MJD (UTC)\n";

  // Query EOP at the probe epoch (need UTC MJD; approximate TT→UTC via MJD).
  const double probe_mjd_utc = probe_tt.to<scales::MJD>().value();
  if (auto eop = eop_at(probe_mjd_utc)) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "DUT1 at probe: " << eop->ut1_minus_utc << " s\n";
    if (eop->pm_xp_arcsec)
      std::cout << "PM Xp        : " << *eop->pm_xp_arcsec << " arcsec\n";
    std::cout << "observed     : " << std::boolalpha << eop->ut1_observed << "\n";
  }

  return 0;
}
