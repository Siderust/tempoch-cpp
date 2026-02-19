#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

using namespace tempoch;

// ============================================================================
// JulianDate
// ============================================================================

TEST(Time, JulianDateJ2000) {
    auto jd = JulianDate::J2000();
    EXPECT_DOUBLE_EQ(jd.value(), 2451545.0);
}

TEST(Time, JulianDateFromUtc) {
    auto jd = JulianDate::from_utc({2000, 1, 1, 12, 0, 0});
    EXPECT_NEAR(jd.value(), 2451545.0, 0.001);
}

TEST(Time, JulianDateRoundtripUtc) {
    UTC original(2026, 7, 15, 22, 0, 0);
    auto jd  = JulianDate::from_utc(original);
    auto utc = jd.to_utc();
    EXPECT_EQ(utc.year, 2026);
    EXPECT_EQ(utc.month, 7);
    EXPECT_EQ(utc.day, 15);
    EXPECT_NEAR(utc.hour, 22, 1);
}

TEST(Time, JulianDateArithmetic) {
    auto jd1 = JulianDate(2451545.0);
    auto jd2 = jd1 + 365.25;
    EXPECT_NEAR(jd2 - jd1, 365.25, 1e-10);
}

TEST(Time, JulianCenturies) {
    auto jd = JulianDate::J2000();
    EXPECT_NEAR(jd.julian_centuries(), 0.0, 1e-10);
}

TEST(Time, JulianDateComparisons) {
    auto a = JulianDate(2451545.0);
    auto b = JulianDate(2451546.0);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a == JulianDate(2451545.0));
    EXPECT_TRUE(a != b);
}

// ============================================================================
// MJD
// ============================================================================

TEST(Time, MjdFromJd) {
    auto jd  = JulianDate::J2000();
    auto mjd = MJD::from_jd(jd);
    EXPECT_NEAR(mjd.value(), jd.to_mjd(), 1e-10);
}

TEST(Time, MjdRoundtrip) {
    auto mjd1 = MJD(60200.0);
    auto jd   = mjd1.to_jd();
    auto mjd2 = MJD::from_jd(jd);
    EXPECT_NEAR(mjd1.value(), mjd2.value(), 1e-10);
}

TEST(Time, MjdFromUtc) {
    auto mjd = MJD::from_utc({2026, 7, 15, 12, 0, 0});
    auto utc = mjd.to_utc();
    EXPECT_EQ(utc.year, 2026);
    EXPECT_EQ(utc.month, 7);
    EXPECT_EQ(utc.day, 15);
}

TEST(Time, MjdArithmetic) {
    auto mjd1 = MJD(60200.0);
    auto mjd2 = mjd1 + 1.5;
    EXPECT_NEAR(mjd2 - mjd1, 1.5, 1e-10);
}

TEST(Time, MjdComparisons) {
    auto a = MJD(60200.0);
    auto b = MJD(60201.0);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a >= a);
    EXPECT_TRUE(a == MJD(60200.0));
    EXPECT_TRUE(a != b);
}

// ============================================================================
// UTC
// ============================================================================

TEST(Time, UtcDefaults) {
    UTC utc;
    EXPECT_EQ(utc.year, 2000);
    EXPECT_EQ(utc.month, 1);
    EXPECT_EQ(utc.day, 1);
    EXPECT_EQ(utc.hour, 12);
}

TEST(Time, UtcToCRoundtrip) {
    UTC utc(2026, 3, 14, 9, 26, 53, 589);
    auto c = utc.to_c();
    auto utc2 = UTC::from_c(c);
    EXPECT_EQ(utc2.year, 2026);
    EXPECT_EQ(utc2.month, 3);
    EXPECT_EQ(utc2.day, 14);
    EXPECT_EQ(utc2.hour, 9);
    EXPECT_EQ(utc2.minute, 26);
    EXPECT_EQ(utc2.second, 53);
    EXPECT_EQ(utc2.nanosecond, 589u);
}
