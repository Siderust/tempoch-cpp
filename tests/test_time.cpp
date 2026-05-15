#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>

#include <type_traits>

using namespace tempoch;

TEST(Time, SplitRoundtripThroughFormats) {
  auto tt = Time<scale::TT>::from_split_seconds(qtty::Second(123456.0), qtty::Second(0.125));

  auto jd = tt.to<format::JD>();
  auto mjd = tt.to<format::MJD>();
  auto j2000 = tt.to<format::J2000s>();

  auto jd_back = jd.to_time();
  auto mjd_back = mjd.to_time();
  auto j2000_back = j2000.to_time();

  EXPECT_NEAR((jd_back - tt).value(), 0.0, 2e-5);
  EXPECT_NEAR((mjd_back - tt).value(), 0.0, 5e-7);
  EXPECT_NEAR((j2000_back - tt).value(), 0.0, 1e-12);
}

TEST(Time, UnixAndGpsRemainSecondFormats) {
  auto utc = Time<scale::UTC>::from_civil({1970, 1, 1, 0, 0, 0});
  auto unix = utc.to<format::Unix>();
  EXPECT_NEAR(unix.value(), 9.43e-06, 1e-9);

  auto unix_back = unix.to_time().to_civil();
  EXPECT_EQ(unix_back.year, 1970);
  EXPECT_EQ(unix_back.month, 1);
  EXPECT_EQ(unix_back.day, 1);

  auto tai = utc.to<scale::TAI>();
  auto gps = tai.to<format::GPS>();
  auto gps_back = gps.to_time();
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
  auto far_future = JulianDate<scale::TT>(2'500'000.0).to_time();
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
