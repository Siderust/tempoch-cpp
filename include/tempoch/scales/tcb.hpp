#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct TCB {};
} // namespace scale

template <> struct is_scale<scale::TCB> : std::true_type {};

template <> struct ScaleTraits<scale::TCB> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TCB;
  static constexpr const char *name() { return "TCB"; }
};

} // namespace tempoch
