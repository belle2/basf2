/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConfig.h>
#include <framework/logging/LogMessage.h>
#include <framework/logging/LogSystem.h>
#include <framework/logging/LogVariableStream.h>


/**
 * \def _B2_DO_NOTHING()
 * Used when things are compiled out.
 */
#define _B2_DO_NOTHING do { } while(false)

/**
 * \def PACKAGENAME()
 * Returns the current package name (set by scons).
 */
#ifdef _PACKAGE_
#define PACKAGENAME() _PACKAGE_
#else
#define PACKAGENAME() nullptr
#endif

/**
 * \def FUNCTIONNAME()
 * Returns the current function name (if supported).
 */
#if defined(__GNUC__)
#define FUNCTIONNAME() __PRETTY_FUNCTION__
#elif defined(__ICC) || defined(__ECC) || defined(__APPLE__)
#define FUNCTIONNAME() __FUNCTION__
#else
#define FUNCTIONNAME() "???"
#endif

/** Send generic log message which uses the << operator to provide text and variables */
#define _B2LOGMESSAGE(loglevel, debuglevel, logTextAndVariables, package, function, file, line) { \
    LogVariableStream varStream; varStream << logTextAndVariables; \
    Belle2::LogSystem::Instance().sendMessage(Belle2::LogMessage(loglevel, std::move(varStream), package, function, file, line, debuglevel)); \
  }

/** send generic log message via the << operator if the log level is enabled. */
#define _B2LOGMESSAGE_IFENABLED(loglevel, debuglevel, logTextAndVariables, package, function, file, line) do { \
    if (Belle2::LogSystem::Instance().isLevelEnabled(loglevel, debuglevel, package)) { \
      _B2LOGMESSAGE(loglevel, debuglevel, logTextAndVariables, package, function, file, line); \
    } } while(false)

/** send generic log message if the log level is enabled. */
#define B2LOG(loglevel, debuglevel, streamText) \
  _B2LOGMESSAGE_IFENABLED(loglevel, debuglevel, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)

/**
 * \def B2DEBUG(level, streamText)
 * Send a debug message, use lower values for level for more important messages.
 */
#ifdef LOG_NO_B2DEBUG
#define B2DEBUG(level, streamText) _B2_DO_NOTHING
#else
#define B2DEBUG(level, streamText) do { if (Belle2::LogSystem::debugEnabled()) B2LOG(Belle2::LogConfig::c_Debug, level, streamText); } while(false)
#endif

/**
 * \def B2DEBUG_MODIFIABLE(level, streamText)
 * Send a debug message for which the level may be adjusted with LogModRealm.
 * For performance reasons normal B2DEBUG messages are ignored if the log level is not c_Debug, even if a LogModRealm is used. This optimization is not done for B2DEBUG_MODIFIABLE. So only use B2DEBUG_MODIFIABLE in conjuction with LogModRealm.
 */
#define B2DEBUG_MODIFIABLE(level, streamText) B2DEBUG(Belle2::LogConfig::c_Debug, level, streamText)


/**
 * \def B2INFO(streamText)
 * Send an info message.
 */
#ifdef LOG_NO_B2INFO
#define B2INFO(streamText) _B2_DO_NOTHING
#else
#define B2INFO(streamText) B2LOG(Belle2::LogConfig::c_Info, 0, streamText)
#endif

/**
 * \def B2RESULT(streamText)
 * Send result message (meant for or informational summary messages, e.g. about number of selected events).
 */
#ifdef LOG_NO_B2RESULT
#define B2RESULT(streamText) _B2_DO_NOTHING
#else
#define B2RESULT(streamText) B2LOG(Belle2::LogConfig::c_Result, 0, streamText)
#endif

/**
 * \def B2WARNING(streamText)
 * Send a warning message.
 */
#ifdef LOG_NO_B2WARNING
#define B2WARNING(streamText) _B2_DO_NOTHING
#else
#define B2WARNING(streamText) B2LOG(Belle2::LogConfig::c_Warning, 0, streamText)
#endif

/**
 * \def B2ERROR(streamText)
 * Send an error message.
 */
#define B2ERROR(streamText) do { \
    _B2LOGMESSAGE(Belle2::LogConfig::c_Error, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__) \
  } while(false)


/**
 * \def B2FATAL(streamText)
 * Send a fatal message, program will abort after printing it.
 */
#define B2FATAL(streamText) do { \
    _B2LOGMESSAGE(Belle2::LogConfig::c_Fatal, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__); \
    exit(1); \
  } while(false)

#ifdef LOG_NO_B2ASSERT
#define B2ASSERT(message, condition) _B2_DO_NOTHING
#else
/**
 * if 'condition' is false, abort execution with a B2FATAL.
 * If the LOG_NO_B2ASSERT macro is defined when framework/logging/Logger.h is included (e.g. via -DLOG_NO_B2ASSERT), these are compiled out.
 */
#define B2ASSERT(message, condition) do { \
    if (!(condition)) { \
      B2FATAL(message); \
    } \
  } while(false)
#endif
