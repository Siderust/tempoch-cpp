#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct ET {};
} // namespace scale

template <> struct is_scale<scale::ET> : std::true_type {};

template <> struct ScaleTraits<scale::ET> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_ET;
  static constexpr const char *name() { return "ET"; }
};

} // namespace tempoch
