#pragma once

#include "base.hpp"

namespace tempoch {

namespace format {
struct GPS {};
} // namespace format

template <> struct is_format<format::GPS> : std::true_type {};

template <> struct FormatTraits<format::GPS> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_GPS;
  static constexpr const char *name() { return "GPS"; }
};

} // namespace tempoch
