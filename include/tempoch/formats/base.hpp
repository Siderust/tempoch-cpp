#pragma once

#include "../ffi_core.hpp"
#include <qtty/qtty.hpp>
#include <type_traits>

namespace tempoch {

template <typename T> struct is_format : std::false_type {};
template <typename T> inline constexpr bool is_format_v = is_format<T>::value;

template <typename F> struct FormatTraits;
template <typename F> inline constexpr tempoch_format_tag_t format_tag_v = FormatTraits<F>::ffi_tag;

} // namespace tempoch
