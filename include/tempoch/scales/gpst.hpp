#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct GPST {};
} // namespace scale

template <> struct is_scale<scale::GPST> : std::true_type {};

template <> struct ScaleTraits<scale::GPST> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_GPST;
  static constexpr const char *name() { return "GPST"; }
};

} // namespace tempoch
