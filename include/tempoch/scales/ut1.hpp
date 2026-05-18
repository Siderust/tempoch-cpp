#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct UT1 {};
} // namespace scale

template <> struct is_scale<scale::UT1> : std::true_type {};

template <> struct ScaleTraits<scale::UT1> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_UT1;
  static constexpr const char *name() { return "UT1"; }
};

} // namespace tempoch
