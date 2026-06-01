#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct BDT {};
} // namespace scale

template <> struct is_scale<scale::BDT> : std::true_type {};

template <> struct ScaleTraits<scale::BDT> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_BDT;
  static constexpr const char *name() { return "BDT"; }
};

} // namespace tempoch
