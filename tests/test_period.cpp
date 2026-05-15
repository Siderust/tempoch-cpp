#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

TEST(Period, GenericMjdTtDuration) {
  using MjdTt = ModifiedJulianDate<scale::TT>;

  Period<MjdTt> period(MjdTt(60200.0), MjdTt(60201.5));
  EXPECT_NEAR(period.duration().value(), 1.5, 1e-12);
  EXPECT_NEAR(period.duration<qtty::Hour>().value(), 36.0, 1e-9);
}

TEST(Period, IntersectionPreservesType) {
  using MjdTt = ModifiedJulianDate<scale::TT>;

  Period<MjdTt> a(MjdTt(60200.0), MjdTt(60202.0));
  Period<MjdTt> b(MjdTt(60201.0), MjdTt(60203.0));
  auto overlap = a.intersection(b);

  EXPECT_NEAR(overlap.start().value(), 60201.0, 1e-12);
  EXPECT_NEAR(overlap.end().value(), 60202.0, 1e-12);
}

TEST(Period, CivilPeriodsUseUtcMjdRoundtrip) {
  Period<CivilTime> period(CivilTime(2026, 1, 1, 0, 0, 0), CivilTime(2026, 1, 2, 0, 0, 0));
  auto start = period.start();
  auto end = period.end();

  EXPECT_EQ(start.year, 2026);
  EXPECT_EQ(start.month, 1);
  EXPECT_EQ(start.day, 1);
  EXPECT_EQ(end.day, 2);
}
