#pragma once

#include "base.hpp"

namespace tempoch {

namespace scale {
struct TDB {};
} // namespace scale

template <> struct is_scale<scale::TDB> : std::true_type {};

template <> struct ScaleTraits<scale::TDB> {
  static constexpr tempoch_scale_tag_t ffi_tag = TEMPOCH_SCALE_TAG_T_TDB;
  static constexpr const char *name() { return "TDB"; }
};

} // namespace tempoch
