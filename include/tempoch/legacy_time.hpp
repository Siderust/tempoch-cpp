#pragma once

/**
 * @file legacy_time.hpp
 * @brief Opt-in legacy aliases for pre-redesign tempoch-cpp call sites.
 *
 * This header is intentionally not included by `tempoch/tempoch.hpp`.
 */

#include "time.hpp"

namespace tempoch::legacy {

using UTC [[deprecated("Use tempoch::CivilTime or tempoch::Time<tempoch::scale::UTC>.")]] =
    CivilTime;
using JulianDate [[deprecated("Use tempoch::JulianDate<tempoch::scale::TT>.")]] =
    tempoch::JulianDate<tempoch::scale::TT>;
using MJD [[deprecated("Use tempoch::ModifiedJulianDate<tempoch::scale::TT>.")]] =
    tempoch::ModifiedJulianDate<tempoch::scale::TT>;
using TT [[deprecated("Use tempoch::Time<tempoch::scale::TT>.")]] = tempoch::Time<tempoch::scale::TT>;
using TAI [[deprecated("Use tempoch::Time<tempoch::scale::TAI>.")]] =
    tempoch::Time<tempoch::scale::TAI>;
using TDB [[deprecated("Use tempoch::Time<tempoch::scale::TDB>.")]] =
    tempoch::Time<tempoch::scale::TDB>;
using TCG [[deprecated("Use tempoch::Time<tempoch::scale::TCG>.")]] =
    tempoch::Time<tempoch::scale::TCG>;
using TCB [[deprecated("Use tempoch::Time<tempoch::scale::TCB>.")]] =
    tempoch::Time<tempoch::scale::TCB>;
using UT [[deprecated("Use tempoch::Time<tempoch::scale::UT1>.")]] = tempoch::Time<tempoch::scale::UT1>;
using JDE [[deprecated("Use tempoch::JulianDate<tempoch::scale::TDB>.")]] =
    tempoch::JulianDate<tempoch::scale::TDB>;
using GPS [[deprecated("Use tempoch::GpsTime.")]] = tempoch::GpsTime;

} // namespace tempoch::legacy
