#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct TAI {};
} // namespace scale

template <> struct is_scale<scale::TAI> : std::true_type {};

template <> struct ScaleTraits<scale::TAI> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TAI;
  static constexpr const char *name() { return "TAI"; }
};

} // namespace tempoch
