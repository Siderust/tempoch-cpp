#pragma once

/**
 * @file time.hpp
 * @brief Public aliases for tempoch time encodings.
 */

#include "time_base.hpp"

namespace tempoch {

template <typename S> using JulianDate = EncodedTime<S, format::JD>;
template <typename S> using ModifiedJulianDate = EncodedTime<S, format::MJD>;
template <typename S> using J2000Seconds = EncodedTime<S, format::J2000s>;

using UnixTime = EncodedTime<scale::UTC, format::Unix>;
using GpsTime = EncodedTime<scale::TAI, format::GPS>;

} // namespace tempoch
