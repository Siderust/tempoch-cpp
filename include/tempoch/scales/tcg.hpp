#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct TCG {};
} // namespace scale

template <> struct is_scale<scale::TCG> : std::true_type {};

template <> struct ScaleTraits<scale::TCG> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TCG;
  static constexpr const char *name() { return "TCG"; }
};

} // namespace tempoch
