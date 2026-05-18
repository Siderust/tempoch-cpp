#pragma once

#include "base.hpp"

namespace tempoch {

namespace format {
struct MJD {};
} // namespace format

template <> struct is_format<format::MJD> : std::true_type {};

template <> struct FormatTraits<format::MJD> {
  using quantity_type = qtty::Day;
  static constexpr tempoch_format_tag_t ffi_tag = TEMPOCH_FORMAT_TAG_T_MJD;
  static constexpr const char *name() { return "MJD"; }
};

} // namespace tempoch
