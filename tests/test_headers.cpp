#include <tempoch/formats/formats.hpp>
#include <tempoch/formats/gps.hpp>
#include <tempoch/formats/j2000s.hpp>
#include <tempoch/formats/jd.hpp>
#include <tempoch/formats/mjd.hpp>
#include <tempoch/formats/unix.hpp>
#include <tempoch/scales/scales.hpp>
#include <tempoch/scales/tai.hpp>
#include <tempoch/scales/tcb.hpp>
#include <tempoch/scales/tcg.hpp>
#include <tempoch/scales/tdb.hpp>
#include <tempoch/scales/tt.hpp>
#include <tempoch/scales/ut1.hpp>
#include <tempoch/scales/utc.hpp>

#include <gtest/gtest.h>
#include <type_traits>

TEST(TempochHeaderLayout, ExposesPerScaleAndFormatTraits) {
  static_assert(tempoch::is_scale_v<tempoch::scale::TT>);
  static_assert(tempoch::is_scale_v<tempoch::scale::TAI>);
  static_assert(tempoch::is_scale_v<tempoch::scale::UTC>);
  static_assert(tempoch::is_scale_v<tempoch::scale::UT1>);
  static_assert(tempoch::is_scale_v<tempoch::scale::TDB>);
  static_assert(tempoch::is_scale_v<tempoch::scale::TCG>);
  static_assert(tempoch::is_scale_v<tempoch::scale::TCB>);

  static_assert(tempoch::is_format_v<tempoch::format::JD>);
  static_assert(tempoch::is_format_v<tempoch::format::MJD>);
  static_assert(tempoch::is_format_v<tempoch::format::J2000s>);
  static_assert(tempoch::is_format_v<tempoch::format::Unix>);
  static_assert(tempoch::is_format_v<tempoch::format::GPS>);

  static_assert(std::is_same_v<typename tempoch::FormatTraits<tempoch::format::JD>::quantity_type,
                               qtty::Day>);
  static_assert(std::is_same_v<
                typename tempoch::FormatTraits<tempoch::format::Unix>::quantity_type,
                qtty::Second>);
}
