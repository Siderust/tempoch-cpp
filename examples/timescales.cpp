// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/**
 * @file timescales.cpp
 * @example timescales.cpp
 * @brief Demonstrates every tempoch time-scale type and cross-scale conversion.
 *
 * Build and run:
 *   cmake -B build && cmake --build build
 *   ./build/timescales
 *
 * Time scales shown:
 *   JulianDate (JD), MJD, TDB, TT, TAI, TCG, TCB, GPS, UT (UT1),
 *   JDE (Julian Ephemeris Date ≡ TDB), UnixTime
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <tempoch/tempoch.hpp>

int main() {
  using namespace tempoch;

  std::cout << "=== tempoch Time-Scale Showcase ===\n\n";

  // ── 1. Reference epoch J2000.0 ─────────────────────────────────────────
  //
  // JD 2 451 545.0 = 2000-01-01T12:00:00 TT.
  // (2000-01-01T11:58:56Z in UTC because ΔT ≈ 63.8 s at this epoch.)
  JulianDate jd = JulianDate(2'451'545.0);
  std::cout << "Reference: J2000.0\n";
  std::cout << "  JD value: " << std::fixed << std::setprecision(6)
            << jd.value() << "\n\n";

  // ── 2. Convert to every scale ───────────────────────────────────────────
  MJD mjd = jd.to<MJDScale>();
  JDE jde = jd.to<JDEScale>();
  TT tt = jd.to<TTScale>();
  TAI tai = jd.to<TAIScale>();
  TDB tdb = jd.to<TDBScale>();
  TCG tcg = jd.to<TCGScale>();
  TCB tcb = jd.to<TCBScale>();
  GPS gps = jd.to<GPSScale>();
  UT ut = jd.to<UTScale>();
  UnixTime ux = jd.to<UnixTimeScale>();

  std::cout << "Scale conversions at J2000.0:\n";
  std::cout << std::fixed << std::setprecision(9);
  std::cout << "  JD   (JD 2451545.0)            : " << jd.value() << "\n";
  std::cout << "  MJD  (JD − 2400000.5)          : " << mjd.value() << "\n";
  std::cout << "  JDE  (≡ TDB, as JD)            : " << jde.value() << "\n";
  std::cout << "  TT   (TAI + 32.184 s)          : " << tt.value() << "\n";
  std::cout << "  TAI  (TT  − 32.184 s)          : " << tai.value() << "\n";
  std::cout << "  TDB  (≈ TT, periodic correction): " << tdb.value() << "\n";
  std::cout << "  TCG  (TT + secular rate)       : " << tcg.value() << "\n";
  std::cout << "  TCB  (TDB + secular rate)      : " << tcb.value() << "\n";
  std::cout << "  GPS  (TAI − 19 s)              : " << gps.value() << "\n";
  std::cout << "  UT1  (UTC + ΔT_earth)          : " << ut.value() << "\n";
  std::cout << std::scientific << std::setprecision(3);
  std::cout << "  Unix (seconds since 1970-01-01): " << ux.value() << "\n\n";

  // ── 3. Fixed offsets between scales (seconds) ──────────────────────────
  constexpr double SECS_PER_DAY = 86'400.0;
  double tt_tai_s = (tt.value() - tai.value()) * SECS_PER_DAY;
  double tai_gps_s = (tai.value() - gps.value()) * SECS_PER_DAY;

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Fixed offsets between scales:\n";
  std::cout << "  TT − TAI  = " << tt_tai_s << " s  (constant 32.184 s)\n";
  std::cout << "  TAI − GPS = " << tai_gps_s
            << " s  (constant 19 s since 1980)\n\n";

  // ── 4. Round-trips ──────────────────────────────────────────────────────
  JulianDate jd_from_mjd = mjd.to<JDScale>();
  JulianDate jd_from_tt = tt.to<JDScale>();
  JulianDate jd_from_tai = tai.to<JDScale>();
  JulianDate jd_from_tdb = tdb.to<JDScale>();

  std::cout << std::scientific << std::setprecision(3);
  std::cout << "Round-trip JD → X → JD residuals (days):\n";
  std::cout << "  JD → MJD → JD : "
            << std::abs(jd_from_mjd.value() - jd.value()) << "\n";
  std::cout << "  JD → TT  → JD : " << std::abs(jd_from_tt.value() - jd.value())
            << "\n";
  std::cout << "  JD → TAI → JD : "
            << std::abs(jd_from_tai.value() - jd.value()) << "\n";
  std::cout << "  JD → TDB → JD : "
            << std::abs(jd_from_tdb.value() - jd.value()) << "\n\n";

  // ── 5. Observable epoch (civil time → all scales) ──────────────────────
  UTC obs(2026, 7, 15, 22, 0, 0);
  JulianDate jd_obs = JulianDate::from_utc(obs);
  MJD mjd_obs = jd_obs.to<MJDScale>();
  TT tt_obs = jd_obs.to<TTScale>();
  TAI tai_obs = jd_obs.to<TAIScale>();
  TDB tdb_obs = jd_obs.to<TDBScale>();

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Observation: 2026-07-15 22:00:00 UTC\n";
  std::cout << "  JD   : " << jd_obs.value() << "\n";
  std::cout << "  MJD  : " << mjd_obs.value() << "\n";
  std::cout << "  TT   : " << tt_obs.value() << "\n";
  std::cout << "  TAI  : " << tai_obs.value() << "\n";
  std::cout << "  TDB  : " << tdb_obs.value() << "\n";
  std::cout << "  Julian centuries since J2000: " << jd_obs.julian_centuries()
            << "\n\n";

  // ── 6. Cross-scale arithmetic: add 1 hour in GPS, round-trip to JD ─────
  constexpr double ONE_HOUR_DAYS = 1.0 / 24.0;
  GPS gps_obs = jd_obs.to<GPSScale>();
  GPS gps_obs_plus1h(gps_obs.value() + ONE_HOUR_DAYS);
  JulianDate jd_plus1h = gps_obs_plus1h.to<JDScale>();

  std::cout << "GPS +1 h → JD difference: " << std::fixed
            << std::setprecision(6)
            << (jd_plus1h.value() - jd_obs.value()) * 24.0 << " h\n";

  std::cout << "\n=== All time-scale examples completed successfully ===\n";
  return 0;
}
