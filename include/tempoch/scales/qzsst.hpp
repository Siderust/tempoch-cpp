#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct QZSST {};
} // namespace scale

template <> struct is_scale<scale::QZSST> : std::true_type {};

template <> struct ScaleTraits<scale::QZSST> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_QZSST;
  static constexpr const char *name() { return "QZSST"; }
};

} // namespace tempoch
