#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct UTC {};
} // namespace scale

template <> struct is_scale<scale::UTC> : std::true_type {};

template <> struct ScaleTraits<scale::UTC> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_UTC;
  static constexpr const char *name() { return "UTC"; }
};

} // namespace tempoch
