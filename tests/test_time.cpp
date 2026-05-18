#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

#include <type_traits>

using namespace tempoch;

TEST(Time, SplitRoundtripThroughFormats) {
  auto tt = Time<scale::TT>::from_split_seconds(qtty::Second(123456.0), qtty::Second(0.125));

  auto jd = tt.to<format::JD>();
  auto mjd = tt.to<format::MJD>();
  auto j2000 = tt.to<format::J2000s>();

  auto jd_back = Time<scale::TT>::from_encoded(jd);
  auto mjd_back = Time<scale::TT>::from_encoded(mjd);
  auto j2000_back = Time<scale::TT>::from_encoded(j2000);

  EXPECT_NEAR((jd_back - tt).value(), 0.0, 2e-5);
  EXPECT_NEAR((mjd_back - tt).value(), 0.0, 5e-7);
  EXPECT_NEAR((j2000_back - tt).value(), 0.0, 1e-12);
}

TEST(Time, UnixAndGpsRemainSecondFormats) {
  auto utc = Time<scale::UTC>::from_civil({1970, 1, 1, 0, 0, 0});
  auto unix = utc.to<format::Unix>();
  EXPECT_NEAR(unix.value(), 9.43e-06, 1e-9);

  auto unix_back = Time<scale::UTC>::from_encoded(unix).to_civil();
  EXPECT_EQ(unix_back.year, 1970);
  EXPECT_EQ(unix_back.month, 1);
  EXPECT_EQ(unix_back.day, 1);

  auto tai = utc.to<scale::TAI>();
  auto gps = tai.to<format::GPS>();
  auto gps_back = Time<scale::TAI>::from_encoded(gps);
  EXPECT_NEAR((gps_back - tai).value(), 0.0, 1e-9);
}

TEST(Time, JulianDateTypesAreDistinct) {
  static_assert(!std::is_same_v<JulianDate<scale::TT>, JulianDate<scale::UT1>>);
  static_assert(!std::is_same_v<JulianDate<scale::TT>, JulianDate<scale::UTC>>);
  static_assert(!std::is_same_v<ModifiedJulianDate<scale::TT>, ModifiedJulianDate<scale::UT1>>);
}

TEST(Time, CivilUtcRoundtrip) {
  CivilTime original(2026, 7, 15, 22, 0, 0);
  auto utc = Time<scale::UTC>::from_civil(original);
  auto roundtrip = utc.to_civil();

  EXPECT_EQ(roundtrip.year, 2026);
  EXPECT_EQ(roundtrip.month, 7);
  EXPECT_EQ(roundtrip.day, 15);
  EXPECT_EQ(roundtrip.hour, 22);
  EXPECT_EQ(roundtrip.minute, 0);
  EXPECT_EQ(roundtrip.second, 0);
}

TEST(Time, PreDefinitionUtcRequiresOptIn) {
  CivilTime pre_definition(1900, 1, 1, 0, 0, 0);
  EXPECT_THROW(Time<scale::UTC>::from_civil(pre_definition), ConversionFailedError);

  auto ctx = TimeContext().allow_pre_definition_utc();
  EXPECT_NO_THROW({
    auto utc = Time<scale::UTC>::from_civil(pre_definition, ctx);
    auto civil = utc.to_civil(ctx);
    EXPECT_EQ(civil.year, 1900);
  });
}

TEST(Time, Ut1HorizonErrorsSurfaceThroughContextBackedConversion) {
  auto far_future = Time<scale::TT>::from_encoded(JulianDate<scale::TT>(2'500'000.0));
  auto ctx = TimeContext::with_builtin_eop();
  EXPECT_THROW((far_future.to_with<scale::UT1>(ctx)), Ut1HorizonExceededError);
}

TEST(Time, ArithmeticIsAffineAndSecondBased) {
  auto tt = Time<scale::TT>::from_raw_j2000_seconds(qtty::Second(10.0));
  auto later = tt + qtty::Hour(1.0);
  auto diff = later - tt;

  EXPECT_NEAR(diff.to<qtty::Second>().value(), 3600.0, 1e-9);
  EXPECT_NEAR(tt.split_seconds().first.value(), 10.0, 1e-12);
}

TEST(Time, TtJulianDateConvenienceUtcRoundtrip) {
  auto jd = JulianDate<scale::TT>::from_utc({2026, 7, 15, 22, 0, 0});
  auto utc = jd.to_utc();

  EXPECT_EQ(utc.year, 2026);
  EXPECT_EQ(utc.month, 7);
  EXPECT_EQ(utc.day, 15);
  EXPECT_NEAR(utc.hour, 22, 1);
}

TEST(Time, TtMjdConvenienceUtcRoundtrip) {
  auto mjd = ModifiedJulianDate<scale::TT>::from_utc({2026, 7, 15, 22, 0, 0});
  auto utc = mjd.to_utc();

  EXPECT_EQ(utc.year, 2026);
  EXPECT_EQ(utc.month, 7);
  EXPECT_EQ(utc.day, 15);
  EXPECT_NEAR(utc.hour, 22, 1);
}

TEST(Time, EncodedDateArithmeticSupportsAssignOperators) {
  auto jd = JulianDate<scale::TT>::J2000();
  jd += qtty::Hour(24.0);
  jd -= qtty::Hour(12.0);

  EXPECT_NEAR(jd.jd_value(), 2451545.5, 1e-10);

  auto mjd = jd.to_mjd();
  mjd += qtty::Minute(720.0);
  EXPECT_NEAR(mjd.to_jd().jd_value(), 2451546.0, 1e-10);
}

TEST(Time, EncodedDateHelpersExposeExpectedValues) {
  auto jd = JulianDate<scale::TT>::J2000();
  auto next = jd + qtty::Day(36525.0);
  auto mjd = ModifiedJulianDate<scale::TT>::from_jd(jd);

  EXPECT_NEAR(jd.julian_centuries(), 0.0, 1e-12);
  EXPECT_NEAR(next.julian_centuries(), 1.0, 1e-12);
  EXPECT_NEAR(mjd.mjd_value(), jd.jd_value() - 2400000.5, 1e-12);
  EXPECT_NEAR(JulianDate<scale::TT>::from_mjd(mjd).jd_value(), jd.jd_value(), 1e-12);
}

TEST(Time, EncodedDateMinMaxAndMeanRemainAvailable) {
  auto a = JulianDate<scale::TT>::J2000();
  auto b = a + qtty::Day(2.0);

  EXPECT_EQ(a.min(b), a);
  EXPECT_EQ(a.max(b), b);
  EXPECT_NEAR(a.mean(b).jd_value(), a.jd_value() + 1.0, 1e-12);
}
