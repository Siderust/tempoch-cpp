#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct TT {};
} // namespace scale

template <> struct is_scale<scale::TT> : std::true_type {};

template <> struct ScaleTraits<scale::TT> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TT;
  static constexpr const char *name() { return "TT"; }
};

} // namespace tempoch
