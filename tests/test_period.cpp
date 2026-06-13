#include <gtest/gtest.h>
#include <tempoch/tempoch.hpp>
#include <type_traits>

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

TEST(Period, TimeScaleConversionConvertsEndpoints) {
  auto start = Time<scale::UTC>::from_civil({2026, 1, 1, 0, 0, 0});
  auto end = Time<scale::UTC>::from_civil({2026, 1, 2, 0, 0, 0});
  Period p(start, end);

  auto out = p.to<scale::TT>();

  static_assert(std::is_same_v<decltype(out), Period<Time<scale::TT>>>);
  EXPECT_NEAR((out.start() - p.start().to<scale::TT>()).value(), 0.0, 1e-5);
  EXPECT_NEAR((out.end() - p.end().to<scale::TT>()).value(), 0.0, 1e-5);
}

TEST(Period, TimeScaleAndFormatConversionConvertsEndpoints) {
  auto start = Time<scale::UTC>::from_civil({2026, 1, 1, 0, 0, 0});
  auto end = Time<scale::UTC>::from_civil({2026, 1, 2, 0, 0, 0});
  Period p(start, end);

  auto out = p.to<scale::TT, format::MJD>();

  static_assert(std::is_same_v<decltype(out), Period<EncodedTime<scale::TT, format::MJD>>>);
  EXPECT_DOUBLE_EQ(out.start().value(), (p.start().to<scale::TT, format::MJD>().value()));
  EXPECT_DOUBLE_EQ(out.end().value(), (p.end().to<scale::TT, format::MJD>().value()));
}

TEST(Period, EncodedScaleAndFormatConversionConvertsEndpoints) {
  using UTCJD = EncodedTime<scale::UTC, format::JD>;
  Period<UTCJD> p(UTCJD(2460000.0), UTCJD(2460001.0));

  auto out = p.to<scale::TT, format::MJD>();

  static_assert(std::is_same_v<decltype(out), Period<EncodedTime<scale::TT, format::MJD>>>);
  EXPECT_DOUBLE_EQ(out.start().value(), (p.start().to<scale::TT, format::MJD>().value()));
  EXPECT_DOUBLE_EQ(out.end().value(), (p.end().to<scale::TT, format::MJD>().value()));
}

TEST(Period, EncodedFormatConversionConvertsEndpoints) {
  using UTCJD = EncodedTime<scale::UTC, format::JD>;
  Period<UTCJD> p(UTCJD(2460000.0), UTCJD(2460001.0));

  auto out = p.to<format::MJD>();

  static_assert(std::is_same_v<decltype(out), Period<EncodedTime<scale::UTC, format::MJD>>>);
  EXPECT_DOUBLE_EQ(out.start().value(), p.start().to<format::MJD>().value());
  EXPECT_DOUBLE_EQ(out.end().value(), p.end().to<format::MJD>().value());
}

TEST(Period, CivilPeriodConvertsFromUtcEndpoints) {
  UTCPeriod p(CivilTime(2026, 1, 1, 0, 0, 0), CivilTime(2026, 1, 2, 0, 0, 0));

  auto out = p.to<scale::TT, format::MJD>();

  static_assert(std::is_same_v<decltype(out), Period<EncodedTime<scale::TT, format::MJD>>>);
  EXPECT_DOUBLE_EQ(out.start().value(),
                   (Time<scale::UTC>::from_civil(p.start()).to<scale::TT, format::MJD>().value()));
  EXPECT_DOUBLE_EQ(out.end().value(),
                   (Time<scale::UTC>::from_civil(p.end()).to<scale::TT, format::MJD>().value()));
}

TEST(Period, ToWithUsesProvidedContextForBothEndpoints) {
  auto ctx = TimeContext::with_builtin_eop();
  auto start = Time<scale::UTC>::from_civil({2026, 1, 1, 0, 0, 0});
  auto end = Time<scale::UTC>::from_civil({2026, 1, 2, 0, 0, 0});
  Period p(start, end);

  auto ut1 = p.to_with<scale::UT1>(ctx);
  auto ut1_mjd = p.to_with<scale::UT1, format::MJD>(ctx);

  static_assert(std::is_same_v<decltype(ut1), Period<Time<scale::UT1>>>);
  static_assert(std::is_same_v<decltype(ut1_mjd), Period<EncodedTime<scale::UT1, format::MJD>>>);
  EXPECT_NEAR((ut1.start() - p.start().to_with<scale::UT1>(ctx)).value(), 0.0, 1e-5);
  EXPECT_NEAR((ut1.end() - p.end().to_with<scale::UT1>(ctx)).value(), 0.0, 1e-5);
  EXPECT_DOUBLE_EQ(ut1_mjd.start().value(),
                   (p.start().to_with<scale::UT1, format::MJD>(ctx).value()));
  EXPECT_DOUBLE_EQ(ut1_mjd.end().value(), (p.end().to_with<scale::UT1, format::MJD>(ctx).value()));
}
