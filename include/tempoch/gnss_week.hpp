#pragma once

/**
 * @file gnss_week.hpp
 * @brief GNSS week-number decomposition mirroring `tempoch::GnssWeek`.
 *
 * Provides the `GnssWeek` value type plus `to_gnss_week` / `from_gnss_week`
 * free functions, defined only for the GNSS coordinate scales (`GPST`, `GST`,
 * `BDT`, `QZSST`), matching the `Time::<S>::to_gnss_week` /
 * `Time::<S>::from_gnss_week` conversions on the Rust side.
 */

#include "scales/scales.hpp"
#include "time_base.hpp"

#include <cstdint>
#include <type_traits>

namespace tempoch {

/// Trait marking the GNSS coordinate scales that support week decomposition.
template <typename S> struct is_gnss_scale : std::false_type {};
template <> struct is_gnss_scale<scale::GPST> : std::true_type {};
template <> struct is_gnss_scale<scale::GST> : std::true_type {};
template <> struct is_gnss_scale<scale::BDT> : std::true_type {};
template <> struct is_gnss_scale<scale::QZSST> : std::true_type {};

template <typename S> inline constexpr bool is_gnss_scale_v = is_gnss_scale<S>::value;

/// Decomposed GNSS week-number form since the constellation's defined epoch.
///
/// Mirrors `tempoch::GnssWeek`. `week` is full (no rollover applied),
/// `seconds_of_week` lies in `[0, 604'800)` and `subsecond_nanos` in
/// `[0, 1'000'000'000)`.
struct GnssWeek {
  /// Full week number since the constellation's epoch (no rollover applied).
  std::uint32_t week;
  /// Seconds since the start of `week`, in `[0, 604'800)`.
  std::uint32_t seconds_of_week;
  /// Subsecond nanoseconds remainder, in `[0, 1'000'000'000)`.
  std::uint32_t subsecond_nanos;
};

/// Decompose a GNSS-scale instant into its week-number form.
template <typename S, std::enable_if_t<is_gnss_scale_v<S>, int> = 0>
inline GnssWeek to_gnss_week(const Time<S> &time) {
  TempochGnssWeek raw{};
  check_status(
      tempoch_time_to_gnss_week(time.c_inner(), static_cast<int32_t>(scale_tag_v<S>), &raw),
      "tempoch::to_gnss_week");
  return GnssWeek{raw.week, raw.seconds_of_week, raw.subsecond_nanos};
}

/// Build a GNSS-scale instant from a week-number decomposition.
template <typename S, std::enable_if_t<is_gnss_scale_v<S>, int> = 0>
inline Time<S> from_gnss_week(const GnssWeek &gw) {
  TempochGnssWeek raw{gw.week, gw.seconds_of_week, gw.subsecond_nanos};
  tempoch_time_t out{};
  check_status(tempoch_time_from_gnss_week(raw, static_cast<int32_t>(scale_tag_v<S>), &out),
               "tempoch::from_gnss_week");
  return Time<S>::from_split_seconds(qtty::Second(out.hi_seconds), qtty::Second(out.lo_seconds));
}

} // namespace tempoch
