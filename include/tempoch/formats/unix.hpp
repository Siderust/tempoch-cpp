#pragma once

#include "base.hpp"

namespace tempoch {

namespace format {
struct Unix {};
} // namespace format

template <> struct is_format<format::Unix> : std::true_type {};

template <> struct FormatTraits<format::Unix> {
  using quantity_type = qtty::Second;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_UNIX;
  static constexpr const char *name() { return "Unix"; }
};

} // namespace tempoch
