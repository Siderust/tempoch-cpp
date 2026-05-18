#pragma once

#include "../ffi_core.hpp"
#include <type_traits>

namespace tempoch {

template <typename T> struct is_scale : std::false_type {};
template <typename T> inline constexpr bool is_scale_v = is_scale<T>::value;

template <typename S> struct ScaleTraits;
template <typename S> inline constexpr tempoch_scale_tag_t scale_tag_v = ScaleTraits<S>::ffi_tag;

} // namespace tempoch
