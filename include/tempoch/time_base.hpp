#pragma once

/**
 * @file time_base.hpp
 * @brief Core `Time<S>`, `EncodedTime<S, F>`, and `TimeContext` types.
 */

#include "civil_time.hpp"
#include "ffi_core.hpp"
#include "formats/formats.hpp"
#include "scales/scales.hpp"
#include <cmath>
#include <memory>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace tempoch {

template <typename S> class Time;
template <typename S, typename F> class EncodedTime;
template <typename T> struct TimeTraits;

namespace detail {

struct ContextDeleter {
  void operator()(tempoch_context_t *ptr) const noexcept { tempoch_context_free(ptr); }
};

inline std::shared_ptr<tempoch_context_t> make_default_context() {
  tempoch_context_t *raw = nullptr;
  check_status(tempoch_context_create_default(&raw), "tempoch_context_create_default");
  return std::shared_ptr<tempoch_context_t>(raw, ContextDeleter{});
}

inline std::shared_ptr<tempoch_context_t> make_builtin_eop_context() {
  tempoch_context_t *raw = nullptr;
  check_status(tempoch_context_create_with_builtin_eop(&raw),
               "tempoch_context_create_with_builtin_eop");
  return std::shared_ptr<tempoch_context_t>(raw, ContextDeleter{});
}

inline std::shared_ptr<tempoch_context_t>
make_pre_definition_context(const tempoch_context_t *parent) {
  tempoch_context_t *raw = nullptr;
  check_status(tempoch_context_allow_pre_definition_utc(parent, &raw),
               "tempoch_context_allow_pre_definition_utc");
  return std::shared_ptr<tempoch_context_t>(raw, ContextDeleter{});
}

inline tempoch_time_t make_time(double hi_seconds, double lo_seconds) {
  tempoch_time_t out{};
  check_status(tempoch_time_new(hi_seconds, lo_seconds, &out), "tempoch_time_new");
  return out;
}

template <typename From, typename To>
inline tempoch_time_t scale_convert(const tempoch_time_t &value, const tempoch_context_t *ctx) {
  tempoch_time_t out{};
  check_status(tempoch_time_scale_convert(value, static_cast<int32_t>(scale_tag_v<From>),
                                          static_cast<int32_t>(scale_tag_v<To>), ctx, &out),
               "tempoch_time_scale_convert");
  return out;
}

template <typename S, typename F>
inline double encode_time(const tempoch_time_t &value, const tempoch_context_t *ctx) {
  double out = 0.0;
  check_status(tempoch_time_to_format(value, static_cast<int32_t>(scale_tag_v<S>),
                                      static_cast<int32_t>(format_tag_v<F>), ctx, &out),
               "tempoch_time_to_format");
  return out;
}

template <typename S, typename F>
inline tempoch_time_t decode_time(double raw, const tempoch_context_t *ctx) {
  tempoch_time_t out{};
  check_status(tempoch_time_from_format(raw, static_cast<int32_t>(scale_tag_v<S>),
                                        static_cast<int32_t>(format_tag_v<F>), ctx, &out),
               "tempoch_time_from_format");
  return out;
}

inline tempoch_time_t time_from_civil(const CivilTime &civil, const tempoch_context_t *ctx) {
  tempoch_time_t out{};
  check_status(tempoch_time_from_civil(civil.to_c(), ctx, &out), "tempoch_time_from_civil");
  return out;
}

inline CivilTime time_to_civil(const tempoch_time_t &value, const tempoch_context_t *ctx) {
  tempoch_utc_t out{};
  check_status(tempoch_time_to_civil(value, ctx, &out), "tempoch_time_to_civil");
  return CivilTime::from_c(out);
}

template <typename Q> inline tempoch_time_t add_seconds(const tempoch_time_t &value, const Q &qty) {
  tempoch_time_t out{};
  qtty_quantity_t raw{qty.value(), qtty::UnitTraits<typename Q::unit_tag>::unit_id()};
  check_status(tempoch_time_add_seconds(value, raw, &out), "tempoch_time_add_seconds");
  return out;
}

inline qtty::Second difference_seconds(const tempoch_time_t &lhs, const tempoch_time_t &rhs) {
  double out = 0.0;
  check_status(tempoch_time_difference_seconds(lhs, rhs, &out), "tempoch_time_difference_seconds");
  return qtty::Second(out);
}

template <typename F> inline typename FormatTraits<F>::quantity_type quantity_from_raw(double raw) {
  return typename FormatTraits<F>::quantity_type(raw);
}

template <typename F> inline void ensure_finite_encoded(double raw, const char *operation) {
  if (!std::isfinite(raw))
    throw ConversionFailedError(std::string(operation) + " failed: non-finite raw value");
}

} // namespace detail

/**
 * @brief Immutable conversion context for UT1 and historical UTC routes.
 */
class TimeContext {
  std::shared_ptr<tempoch_context_t> handle_;

  explicit TimeContext(std::shared_ptr<tempoch_context_t> handle) : handle_(std::move(handle)) {}

public:
  TimeContext() : handle_(detail::make_default_context()) {}

  static TimeContext with_builtin_eop() { return TimeContext(detail::make_builtin_eop_context()); }

  TimeContext allow_pre_definition_utc() const {
    return TimeContext(detail::make_pre_definition_context(handle_.get()));
  }

  const tempoch_context_t *get() const noexcept { return handle_.get(); }
};

/**
 * @brief A point in time on scale @p S, stored as a split J2000-second pair.
 */
template <typename S> class Time {
  static_assert(is_scale_v<S>, "Time<S> requires a valid tempoch::scale tag");

  tempoch_time_t raw_;

  explicit Time(const tempoch_time_t &raw) : raw_(raw) {}

  template <typename> friend class Time;
  template <typename, typename> friend class EncodedTime;

public:
  using scale_type = S;

  Time() : raw_(detail::make_time(0.0, 0.0)) {}

  static Time from_split_seconds(qtty::Second hi, qtty::Second lo = qtty::Second(0.0)) {
    return Time(detail::make_time(hi.value(), lo.value()));
  }

  static Time from_raw_j2000_seconds(qtty::Second seconds) { return from_split_seconds(seconds); }

  /// Decode a scalar encoding @p Fmt into canonical split storage on scale @p S (default context).
  template <typename Fmt> static Time from_encoded(const EncodedTime<S, Fmt> &encoded) {
    return Time(detail::decode_time<S, Fmt>(encoded.value(), nullptr));
  }

  /// Decode using explicit UTC / UT1 policy from @p ctx when required by format @p Fmt.
  template <typename Fmt>
  static Time from_encoded_with(const EncodedTime<S, Fmt> &encoded, const TimeContext &ctx) {
    return Time(detail::decode_time<S, Fmt>(encoded.value(), ctx.get()));
  }

  std::pair<qtty::Second, qtty::Second> split_seconds() const noexcept {
    return {qtty::Second(raw_.hi_seconds), qtty::Second(raw_.lo_seconds)};
  }

  qtty::Second total_seconds() const noexcept {
    return qtty::Second(raw_.hi_seconds + raw_.lo_seconds);
  }

  const tempoch_time_t &c_inner() const noexcept { return raw_; }

  static constexpr const char *label() { return ScaleTraits<S>::name(); }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::UTC>, int> = 0>
  static Time from_civil(const CivilTime &civil) {
    return Time(detail::time_from_civil(civil, nullptr));
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::UTC>, int> = 0>
  static Time from_civil(const CivilTime &civil, const TimeContext &ctx) {
    return Time(detail::time_from_civil(civil, ctx.get()));
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::UTC>, int> = 0>
  CivilTime to_civil() const {
    return detail::time_to_civil(raw_, nullptr);
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::UTC>, int> = 0>
  CivilTime to_civil(const TimeContext &ctx) const {
    return detail::time_to_civil(raw_, ctx.get());
  }

  template <typename TargetScale,
            std::enable_if_t<is_scale_v<TargetScale> && !std::is_same_v<S, scale::UT1> &&
                                 !std::is_same_v<TargetScale, scale::UT1>,
                             int> = 0>
  Time<TargetScale> to() const {
    return Time<TargetScale>(detail::scale_convert<S, TargetScale>(raw_, nullptr));
  }

  template <typename TargetScale,
            std::enable_if_t<is_scale_v<TargetScale> && (std::is_same_v<S, scale::UT1> ||
                                                         std::is_same_v<TargetScale, scale::UT1>),
                             int> = 0>
  Time<TargetScale> to() const = delete;

  template <typename TargetScale, std::enable_if_t<is_scale_v<TargetScale>, int> = 0>
  Time<TargetScale> to_with(const TimeContext &ctx) const {
    return Time<TargetScale>(detail::scale_convert<S, TargetScale>(raw_, ctx.get()));
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  EncodedTime<S, TargetFormat> to() const {
    return EncodedTime<S, TargetFormat>(detail::quantity_from_raw<TargetFormat>(
        detail::encode_time<S, TargetFormat>(raw_, nullptr)));
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  EncodedTime<S, TargetFormat> to_with(const TimeContext &ctx) const {
    return EncodedTime<S, TargetFormat>(detail::quantity_from_raw<TargetFormat>(
        detail::encode_time<S, TargetFormat>(raw_, ctx.get())));
  }

  template <typename TargetScale, std::enable_if_t<is_scale_v<TargetScale>, int> = 0>
  std::optional<Time<TargetScale>> try_to() const {
    try {
      return to_with<TargetScale>(TimeContext());
    } catch (const std::exception &) {
      return std::nullopt;
    }
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  std::optional<EncodedTime<S, TargetFormat>> try_to() const {
    try {
      return to_with<TargetFormat>(TimeContext());
    } catch (const std::exception &) {
      return std::nullopt;
    }
  }

  template <typename Q> Time operator+(const Q &delta) const {
    return Time(detail::add_seconds(raw_, delta));
  }

  template <typename Q> Time operator-(const Q &delta) const { return *this + Q(-delta.value()); }

  template <typename Q> Time &operator+=(const Q &delta) {
    raw_ = detail::add_seconds(raw_, delta);
    return *this;
  }

  template <typename Q> Time &operator-=(const Q &delta) {
    raw_ = detail::add_seconds(raw_, Q(-delta.value()));
    return *this;
  }

  qtty::Second operator-(const Time &other) const {
    return detail::difference_seconds(raw_, other.raw_);
  }

  bool operator==(const Time &other) const noexcept {
    return raw_.hi_seconds == other.raw_.hi_seconds && raw_.lo_seconds == other.raw_.lo_seconds;
  }

  bool operator!=(const Time &other) const noexcept { return !(*this == other); }

  bool operator<(const Time &other) const noexcept {
    return raw_.hi_seconds < other.raw_.hi_seconds ||
           (raw_.hi_seconds == other.raw_.hi_seconds && raw_.lo_seconds < other.raw_.lo_seconds);
  }

  bool operator<=(const Time &other) const noexcept { return !(other < *this); }
  bool operator>(const Time &other) const noexcept { return other < *this; }
  bool operator>=(const Time &other) const noexcept { return !(*this < other); }
};

template <typename S> inline std::ostream &operator<<(std::ostream &os, const Time<S> &time) {
  return os << Time<S>::label() << " " << time.total_seconds().value() << " s";
}

/**
 * @brief A typed external encoding of a time instant on scale @p S.
 */
template <typename S, typename F> class EncodedTime {
  static_assert(is_scale_v<S>, "EncodedTime<S, F> requires a valid tempoch::scale tag");
  static_assert(is_format_v<F>, "EncodedTime<S, F> requires a valid tempoch::format tag");

public:
  using scale_type = S;
  using format_type = F;
  using quantity_type = typename FormatTraits<F>::quantity_type;

private:
  quantity_type raw_;

public:
  EncodedTime() : raw_(0.0) {}

  explicit EncodedTime(quantity_type raw) : raw_(raw) {
    detail::ensure_finite_encoded<F>(raw_.value(), "EncodedTime::EncodedTime");
  }

  explicit EncodedTime(double value) : EncodedTime(quantity_type(value)) {}

  static std::optional<EncodedTime> try_new(quantity_type raw) {
    if (!std::isfinite(raw.value()))
      return std::nullopt;
    return EncodedTime(raw);
  }

  /// Construct directly from a quantity, validating that the value is finite.
  ///
  /// Unlike `try_new`, this throws `TempochException` on non-finite input.
  static EncodedTime from_raw(quantity_type raw) { return EncodedTime(raw); }

  quantity_type raw() const noexcept { return raw_; }
  quantity_type quantity() const noexcept { return raw_; }
  double value() const noexcept { return raw_.value(); }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  static EncodedTime J2000() {
    return EncodedTime(tempoch_const_j2000_jd_tt());
  }

  template <typename TargetScale, std::enable_if_t<is_scale_v<TargetScale>, int> = 0>
  auto to() const {
    return Time<S>::from_encoded(*this).template to<TargetScale>();
  }

  template <typename TargetScale, std::enable_if_t<is_scale_v<TargetScale>, int> = 0>
  Time<TargetScale> to_with(const TimeContext &ctx) const {
    return Time<S>::from_encoded_with(*this, ctx).template to_with<TargetScale>(ctx);
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  EncodedTime<S, TargetFormat> to() const {
    return Time<S>::from_encoded(*this).template to<TargetFormat>();
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  EncodedTime<S, TargetFormat> to_with(const TimeContext &ctx) const {
    return Time<S>::from_encoded_with(*this, ctx).template to_with<TargetFormat>(ctx);
  }

  template <typename TargetScale, std::enable_if_t<is_scale_v<TargetScale>, int> = 0>
  std::optional<Time<TargetScale>> try_to() const {
    try {
      return to_with<TargetScale>(TimeContext());
    } catch (const std::exception &) {
      return std::nullopt;
    }
  }

  template <typename TargetFormat, std::enable_if_t<is_format_v<TargetFormat>, int> = 0>
  std::optional<EncodedTime<S, TargetFormat>> try_to() const {
    try {
      return to_with<TargetFormat>(TimeContext());
    } catch (const std::exception &) {
      return std::nullopt;
    }
  }

  template <typename Q> EncodedTime operator+(const Q &delta) const {
    return (Time<S>::from_encoded(*this) + delta).template to<F>();
  }

  template <typename Q> EncodedTime operator-(const Q &delta) const {
    return (Time<S>::from_encoded(*this) - delta).template to<F>();
  }

  template <typename Q> EncodedTime &operator+=(const Q &delta) {
    *this = *this + delta;
    return *this;
  }

  template <typename Q> EncodedTime &operator-=(const Q &delta) {
    *this = *this - delta;
    return *this;
  }

  quantity_type operator-(const EncodedTime &other) const {
    return (Time<S>::from_encoded(*this) - Time<S>::from_encoded(other))
        .template to<quantity_type>();
  }

  bool operator==(const EncodedTime &other) const noexcept { return raw_ == other.raw_; }
  bool operator!=(const EncodedTime &other) const noexcept { return raw_ != other.raw_; }
  bool operator<(const EncodedTime &other) const noexcept { return raw_ < other.raw_; }
  bool operator<=(const EncodedTime &other) const noexcept { return raw_ <= other.raw_; }
  bool operator>(const EncodedTime &other) const noexcept { return raw_ > other.raw_; }
  bool operator>=(const EncodedTime &other) const noexcept { return raw_ >= other.raw_; }

  EncodedTime min(const EncodedTime &other) const noexcept {
    return *this <= other ? *this : other;
  }
  EncodedTime max(const EncodedTime &other) const noexcept {
    return *this >= other ? *this : other;
  }

  EncodedTime mean(const EncodedTime &other) const {
    return EncodedTime(quantity_type((raw_.value() + other.raw_.value()) * 0.5));
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  double jd_value() const noexcept {
    return raw_.value();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::MJD>, int> = 0>
  double mjd_value() const noexcept {
    return raw_.value();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  qtty::JulianCentury julian_centuries_qty() const noexcept {
    return qtty::JulianCentury((raw_.value() - 2451545.0) / 36525.0);
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  double julian_centuries() const noexcept {
    return julian_centuries_qty().value();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  EncodedTime<S, format::MJD> to_mjd() const {
    return this->template to<format::MJD>();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::MJD>, int> = 0>
  EncodedTime<S, format::JD> to_jd() const {
    return this->template to<format::JD>();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::JD>, int> = 0>
  static EncodedTime from_mjd(const EncodedTime<S, format::MJD> &mjd) {
    return mjd.template to<format::JD>();
  }

  template <typename U = F, std::enable_if_t<std::is_same_v<U, format::MJD>, int> = 0>
  static EncodedTime from_jd(const EncodedTime<S, format::JD> &jd) {
    return jd.template to<format::MJD>();
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::TT>, int> = 0>
  static EncodedTime from_utc(const CivilTime &civil) {
    return Time<scale::UTC>::from_civil(civil).template to<scale::TT>().template to<F>();
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::TT>, int> = 0>
  static EncodedTime from_utc(const CivilTime &civil, const TimeContext &ctx) {
    return Time<scale::UTC>::from_civil(civil, ctx).template to<scale::TT>().template to<F>();
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::TT>, int> = 0>
  CivilTime to_utc() const {
    return Time<S>::from_encoded(*this).template to<scale::UTC>().to_civil();
  }

  template <typename U = S, std::enable_if_t<std::is_same_v<U, scale::TT>, int> = 0>
  CivilTime to_utc(const TimeContext &ctx) const {
    return Time<S>::from_encoded_with(*this, ctx).template to<scale::UTC>().to_civil(ctx);
  }
};

template <typename S, typename F>
inline std::ostream &operator<<(std::ostream &os, const EncodedTime<S, F> &time) {
  return os << ScaleTraits<S>::name() << ' ' << FormatTraits<F>::name() << ' ' << time.raw();
}

} // namespace tempoch
