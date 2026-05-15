#pragma once

/**
 * @file scales.hpp
 * @brief Time-scale and time-format tag types for the tempoch C++ API.
 *
 * The C++ surface follows the Rust `tempoch` model: physical *scale* and
 * external *format* are orthogonal type parameters.
 */

#include "ffi_core.hpp"
#include <qtty/qtty.hpp>
#include <type_traits>

namespace tempoch {

namespace scale {

struct TT {};
struct TAI {};
struct UTC {};
struct UT1 {};
struct TDB {};
struct TCG {};
struct TCB {};

} // namespace scale

namespace format {

struct JD {};
struct MJD {};
struct J2000s {};
struct Unix {};
struct GPS {};

} // namespace format

template <typename T> struct is_scale : std::false_type {};
template <> struct is_scale<scale::TT> : std::true_type {};
template <> struct is_scale<scale::TAI> : std::true_type {};
template <> struct is_scale<scale::UTC> : std::true_type {};
template <> struct is_scale<scale::UT1> : std::true_type {};
template <> struct is_scale<scale::TDB> : std::true_type {};
template <> struct is_scale<scale::TCG> : std::true_type {};
template <> struct is_scale<scale::TCB> : std::true_type {};

template <typename T> inline constexpr bool is_scale_v = is_scale<T>::value;

template <typename T> struct is_format : std::false_type {};
template <> struct is_format<format::JD> : std::true_type {};
template <> struct is_format<format::MJD> : std::true_type {};
template <> struct is_format<format::J2000s> : std::true_type {};
template <> struct is_format<format::Unix> : std::true_type {};
template <> struct is_format<format::GPS> : std::true_type {};

template <typename T> inline constexpr bool is_format_v = is_format<T>::value;

template <typename S> struct ScaleTraits;

template <> struct ScaleTraits<scale::TT> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TT;
  static constexpr const char *name() { return "TT"; }
};

template <> struct ScaleTraits<scale::TAI> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TAI;
  static constexpr const char *name() { return "TAI"; }
};

template <> struct ScaleTraits<scale::UTC> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_UTC;
  static constexpr const char *name() { return "UTC"; }
};

template <> struct ScaleTraits<scale::UT1> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_UT1;
  static constexpr const char *name() { return "UT1"; }
};

template <> struct ScaleTraits<scale::TDB> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TDB;
  static constexpr const char *name() { return "TDB"; }
};

template <> struct ScaleTraits<scale::TCG> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TCG;
  static constexpr const char *name() { return "TCG"; }
};

template <> struct ScaleTraits<scale::TCB> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TCB;
  static constexpr const char *name() { return "TCB"; }
};

template <typename F> struct FormatTraits;

template <> struct FormatTraits<format::JD> {
  using quantity_type = qtty::Day;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_JD;
  static constexpr const char *name() { return "JD"; }
};

template <> struct FormatTraits<format::MJD> {
  using quantity_type = qtty::Day;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_MJD;
  static constexpr const char *name() { return "MJD"; }
};

template <> struct FormatTraits<format::J2000s> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_J2000_SECONDS;
  static constexpr const char *name() { return "J2000s"; }
};

template <> struct FormatTraits<format::Unix> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_UNIX;
  static constexpr const char *name() { return "Unix"; }
};

template <> struct FormatTraits<format::GPS> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_GPS;
  static constexpr const char *name() { return "GPS"; }
};

template <typename S> inline constexpr tempoch_scale_tag_t scale_tag_v = ScaleTraits<S>::ffi_tag;
template <typename F> inline constexpr tempoch_format_tag_t format_tag_v = FormatTraits<F>::ffi_tag;

} // namespace tempoch
