/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include <framework/logging/LogConfig.h>
#include <framework/logging/LogMessage.h>
#include <framework/logging/LogSystem.h>
#include <framework/logging/LogMethod.h>

#include <string>
#include <sstream>

namespace Belle2 {

// package name
#ifdef _PACKAGE_
#define PACKAGENAME() _PACKAGE_
#else
#define PACKAGENAME() "NoPackage"
#endif

// function name
#if defined(__GNUC__)
#define FUNCTIONNAME() __PRETTY_FUNCTION__
#elif defined(__ICC) || defined(__ECC) || defined(__APPLE__)
#define FUNCTIONNAME() __FUNCTION__
#else
#define FUNCTIONNAME() "???"
#endif

// send generic log message
#define B2LOGMESSAGE(loglevel, debuglevel, streamText, package, function, file, line) { \
    std::ostringstream stringBuffer; stringBuffer << streamText;        \
    LogSystem::Instance().sendMessage(LogMessage(loglevel, stringBuffer.rdbuf()->str(), package, function, file, line)); \
  }

// send generic log message if the log level is enabled
#define B2LOGMESSAGE_IFENABLED(loglevel, debuglevel, streamText, package, function, file, line) { \
    if (LogSystem::Instance().isLevelEnabled(loglevel, debuglevel, package)) { \
      B2LOGMESSAGE(loglevel, debuglevel, streamText, package, function, file, line); \
    } }

// debug messages
#ifdef LOG_NO_B2DEBUG
#define B2DEBUG(level, streamText)
#else
#define B2DEBUG(level, streamText) \
  B2LOGMESSAGE_IFENABLED(LogConfig::c_Debug, level, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)
#endif

// info messages
#ifdef LOG_NO_B2INFO
#define B2INFO(streamText)
#else
#define B2INFO(streamText) \
  B2LOGMESSAGE_IFENABLED(LogConfig::c_Info, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)
#endif

// warning messages
#ifdef LOG_NO_B2WARNING
#define B2WARNING(streamText)
#else
#define B2WARNING(streamText) \
  B2LOGMESSAGE_IFENABLED(LogConfig::c_Warning, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)
#endif

// error messages
#define B2ERROR(streamText) \
  B2LOGMESSAGE(LogConfig::c_Error, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)

// fatal messages
#define B2FATAL(streamText) \
  B2LOGMESSAGE(LogConfig::c_Fatal, 0, streamText, PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)

// scoped logging for entering/leaving methods
#ifdef LOG_NO_B2METHOD
#define B2METHOD()
#else
#define B2METHOD() LogMethod logMethod(PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__);
#endif

}  // end namespace Belle2

#endif  // LOGGER_H
