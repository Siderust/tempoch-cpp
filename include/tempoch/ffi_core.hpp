#pragma once

/**
 * @file ffi_core.hpp
 * @brief Error handling for the tempoch C++ wrapper.
 *
 * Maps C-style status codes from tempoch-ffi to a typed C++ exception
 * hierarchy, and provides a check_status helper.
 */

#include <stdexcept>
#include <string>

extern "C" {
#include "tempoch_ffi.h"
}

namespace tempoch {

// ============================================================================
// Exception Hierarchy
// ============================================================================

/**
 * @brief Base exception for all tempoch errors.
 */
class TempochException : public std::runtime_error {
public:
  explicit TempochException(const std::string &msg) : std::runtime_error(msg) {}
};

/**
 * @brief A required output pointer was null.
 */
class NullPointerError : public TempochException {
public:
  explicit NullPointerError(const std::string &msg) : TempochException(msg) {}
};

/**
 * @brief UTC conversion failed (date out of range or invalid).
 */
class UtcConversionError : public TempochException {
public:
  explicit UtcConversionError(const std::string &msg) : TempochException(msg) {}
};

/**
 * @brief The period is invalid (start > end).
 */
class InvalidPeriodError : public TempochException {
public:
  explicit InvalidPeriodError(const std::string &msg) : TempochException(msg) {}
};

/**
 * @brief The two periods do not intersect.
 */
class NoIntersectionError : public TempochException {
public:
  explicit NoIntersectionError(const std::string &msg)
      : TempochException(msg) {}
};

// ============================================================================
// Error Translation
// ============================================================================

/**
 * @brief Check a tempoch_status_t and throw the appropriate exception on error.
 */
inline void check_status(tempoch_status_t status, const char *operation) {
  if (status == TEMPOCH_STATUS_T_OK)
    return;

  std::string msg = std::string(operation) + " failed: ";
  switch (status) {
  case TEMPOCH_STATUS_T_NULL_POINTER:
    throw NullPointerError(msg + "null output pointer");
  case TEMPOCH_STATUS_T_UTC_CONVERSION_FAILED:
    throw UtcConversionError(msg + "UTC conversion failed");
  case TEMPOCH_STATUS_T_INVALID_PERIOD:
    throw InvalidPeriodError(msg + "invalid period (start > end)");
  case TEMPOCH_STATUS_T_NO_INTERSECTION:
    throw NoIntersectionError(msg + "periods do not intersect");
  default:
    throw TempochException(msg + "unknown error (" + std::to_string(status) +
                           ")");
  }
}

} // namespace tempoch
