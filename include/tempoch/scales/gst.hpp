#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct GST {};
} // namespace scale

template <> struct is_scale<scale::GST> : std::true_type {};

template <> struct ScaleTraits<scale::GST> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_GST;
  static constexpr const char *name() { return "GST"; }
};

} // namespace tempoch
