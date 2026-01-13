#ifndef DS_VERSION_H
#define DS_VERSION_H

/**
 * @file version.h
 * @brief Library version information.
 */

/**
 * @addtogroup UTILS
 * @{
 */

/** @name Version Macros */
/** @{ */

/** @brief Major version: Incremented for incompatible API changes. */
#define DS_VERSION_MAJOR 1

/** @brief Minor version: Incremented for backwards-compatible functionality. */
#define DS_VERSION_MINOR 0

/** @brief Patch version: Incremented for backwards-compatible bug fixes. */
#define DS_VERSION_PATCH 0

/** @brief Helper to stringify the version. */
#define DS_STR(x) #x
#define DS_XSTR(x) DS_STR(x)

/** @brief Full version string (e.g., "1.0.0"). */
#define DS_VERSION_STRING \
    DS_XSTR(DS_VERSION_MAJOR) "." \
    DS_XSTR(DS_VERSION_MINOR) "." \
    DS_XSTR(DS_VERSION_PATCH)

/** @} */
/** @} */

#endif // DS_VERSION_H