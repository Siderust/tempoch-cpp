#include <gtest/gtest.h>
#include <qtty/qtty.hpp>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

// ============================================================================
// Period
// ============================================================================

TEST(Period, Duration) {
  Period p(MJD(60200.0), MJD(60201.0));
  EXPECT_NEAR(p.duration().value(), 1.0, 1e-10);
}

TEST(Period, DurationInSeconds) {
  Period p(MJD(60200.0), MJD(60201.0)); // 1 day
  EXPECT_NEAR(p.duration<qtty::Second>().value(), 86400.0, 1e-6);
}

TEST(Period, DurationInHours) {
  Period p(MJD(60200.0), MJD(60201.0)); // 1 day = 24 hours
  EXPECT_NEAR(p.duration<qtty::Hour>().value(), 24.0, 1e-10);
}

TEST(Period, StartEnd) {
  Period p(MJD(60200.0), MJD(60201.0));
  EXPECT_NEAR(p.start().value(), 60200.0, 1e-10);
  EXPECT_NEAR(p.end().value(), 60201.0, 1e-10);
}

TEST(Period, FromMJD) {
  MJD s(60200.0);
  MJD e(60201.0);
  Period p(s, e);
  EXPECT_NEAR(p.duration().value(), 1.0, 1e-10);
}

TEST(Period, Intersection) {
  Period a(MJD(60200.0), MJD(60202.0));
  Period b(MJD(60201.0), MJD(60203.0));
  auto c = a.intersection(b);
  EXPECT_NEAR(c.start().value(), 60201.0, 1e-10);
  EXPECT_NEAR(c.end().value(), 60202.0, 1e-10);
}

TEST(Period, NoIntersectionThrows) {
  Period a(MJD(60200.0), MJD(60201.0));
  Period b(MJD(60202.0), MJD(60203.0));
  EXPECT_THROW(a.intersection(b), NoIntersectionError);
}

TEST(Period, InvalidThrows) {
  EXPECT_THROW((Period(MJD(60203.0), MJD(60200.0))), InvalidPeriodError);
}

TEST(Period, CInner) {
  Period p(MJD(60200.0), MJD(60201.0));
  auto c = p.c_inner();
  EXPECT_NEAR(c.start_mjd, 60200.0, 1e-10);
  EXPECT_NEAR(c.end_mjd, 60201.0, 1e-10);
}

TEST(Period, FromC) {
  tempoch_period_mjd_t c{60200.0, 60201.0};
  auto p = Period<MJD>::from_c(c);
  EXPECT_NEAR(p.start().value(), 60200.0, 1e-10);
  EXPECT_NEAR(p.duration().value(), 1.0, 1e-10);
}

TEST(Period, DurationInMinutes) {
  Period p(MJD(60200.0), MJD(60200.5)); // 0.5 day = 720 minutes
  auto min = p.duration<qtty::Minute>();
  EXPECT_NEAR(min.value(), 720.0, 1e-6);
}
