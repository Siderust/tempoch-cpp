#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

// ============================================================================
// Period
// ============================================================================

TEST(Period, Duration) {
    Period p(60200.0, 60201.0);
    EXPECT_NEAR(p.duration_days(), 1.0, 1e-10);
}

TEST(Period, StartEnd) {
    Period p(60200.0, 60201.0);
    EXPECT_NEAR(p.start_mjd(), 60200.0, 1e-10);
    EXPECT_NEAR(p.end_mjd(), 60201.0, 1e-10);
    EXPECT_NEAR(p.start().value(), 60200.0, 1e-10);
    EXPECT_NEAR(p.end().value(), 60201.0, 1e-10);
}

TEST(Period, FromMJD) {
    MJD s(60200.0);
    MJD e(60201.0);
    Period p(s, e);
    EXPECT_NEAR(p.duration_days(), 1.0, 1e-10);
}

TEST(Period, Intersection) {
    Period a(60200.0, 60202.0);
    Period b(60201.0, 60203.0);
    auto c = a.intersection(b);
    EXPECT_NEAR(c.start_mjd(), 60201.0, 1e-10);
    EXPECT_NEAR(c.end_mjd(), 60202.0, 1e-10);
}

TEST(Period, NoIntersectionThrows) {
    Period a(60200.0, 60201.0);
    Period b(60202.0, 60203.0);
    EXPECT_THROW(a.intersection(b), NoIntersectionError);
}

TEST(Period, InvalidThrows) {
    EXPECT_THROW(Period(60203.0, 60200.0), InvalidPeriodError);
}

TEST(Period, CInner) {
    Period p(60200.0, 60201.0);
    auto c = p.c_inner();
    EXPECT_NEAR(c.start_mjd, 60200.0, 1e-10);
    EXPECT_NEAR(c.end_mjd, 60201.0, 1e-10);
}

TEST(Period, FromC) {
    tempoch_period_mjd_t c{60200.0, 60201.0};
    auto p = Period::from_c(c);
    EXPECT_NEAR(p.start_mjd(), 60200.0, 1e-10);
    EXPECT_NEAR(p.duration_days(), 1.0, 1e-10);
}
