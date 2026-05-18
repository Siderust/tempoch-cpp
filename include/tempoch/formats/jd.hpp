#pragma once

#include "base.hpp"

namespace tempoch {

namespace format {
struct JD {};
} // namespace format

template <> struct is_format<format::JD> : std::true_type {};

template <> struct FormatTraits<format::JD> {
  using quantity_type = qtty::Day;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_JD;
  static constexpr const char *name() { return "JD"; }
};

} // namespace tempoch
