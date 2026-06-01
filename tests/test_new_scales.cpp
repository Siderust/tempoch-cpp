// SPDX-License-Identifier: AGPL-3.0-only
// Copyright (C) 2026 Vallés Puig, Ramon

// Tests for the GNSS and SPICE-compatibility time scales newly mirrored from
// the Rust tempoch crate (ET, GPST, GST, BDT, QZSST).

#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

namespace {

// J2000-second value of an instant on its own scale axis.
template <typename Scale> double j2000s_of(const Time<Scale> &t) {
  return t.template to<format::J2000s>().value();
}

} // namespace

TEST(NewScales, GnssOffsetsFromTai) {
  // Pick an instant well after every GNSS epoch.
  auto tai = Time<scale::TAI>::from_split_seconds(qtty::Second(1.0e9), qtty::Second(0.0));
  double tai_s = j2000s_of(tai);

  // GPST = TAI - 19 s (the GPST axis reads 19 s less than TAI).
  EXPECT_NEAR(tai_s - j2000s_of(tai.to<scale::GPST>()), 19.0, 1e-6);
  // GST and QZSST share the GPST offset.
  EXPECT_NEAR(tai_s - j2000s_of(tai.to<scale::GST>()), 19.0, 1e-6);
  EXPECT_NEAR(tai_s - j2000s_of(tai.to<scale::QZSST>()), 19.0, 1e-6);
  // BDT = TAI - 33 s.
  EXPECT_NEAR(tai_s - j2000s_of(tai.to<scale::BDT>()), 33.0, 1e-6);
}

TEST(NewScales, EtMatchesTdb) {
  auto tdb = Time<scale::TDB>::from_split_seconds(qtty::Second(5.0e8), qtty::Second(0.0));
  // ET is a SPICE-compatibility marker routing through TDB; same axis value.
  EXPECT_NEAR(j2000s_of(tdb.to<scale::ET>()), j2000s_of(tdb), 1e-9);
}

TEST(NewScales, RoundTripThroughGpst) {
  auto tt = Time<scale::TT>::from_split_seconds(qtty::Second(7.5e8), qtty::Second(0.25));
  auto back = tt.to<scale::GPST>().to<scale::TT>();
  EXPECT_NEAR((back - tt).value(), 0.0, 1e-6);
}

TEST(NewScales, BdtIsGpstMinus14) {
  auto tai = Time<scale::TAI>::from_split_seconds(qtty::Second(1.0e9), qtty::Second(0.0));
  double gpst_s = j2000s_of(tai.to<scale::GPST>());
  double bdt_s = j2000s_of(tai.to<scale::BDT>());
  // BDT = GPST - 14 s.
  EXPECT_NEAR(gpst_s - bdt_s, 14.0, 1e-6);
}
