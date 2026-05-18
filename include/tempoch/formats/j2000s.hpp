#pragma once

#include "base.hpp"

namespace tempoch {

namespace format {
struct J2000s {};
} // namespace format

template <> struct is_format<format::J2000s> : std::true_type {};

template <> struct FormatTraits<format::J2000s> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_J2000_SECONDS;
  static constexpr const char *name() { return "J2000s"; }
};

} // namespace tempoch
