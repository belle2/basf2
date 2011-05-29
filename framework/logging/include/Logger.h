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
#include <iostream>
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

// debug messages
#ifdef LOG_NO_B2DEBUG
#define B2DEBUG(level, streamText)
#else
#define B2DEBUG(level, streamText) { if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug,level,PACKAGENAME())) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Debug,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__)); } }
#endif

// info messages
#ifdef LOG_NO_B2INFO
#define B2INFO(streamText)
#else
#define B2INFO(streamText) { if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Info,0,PACKAGENAME())) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Info,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__)); } }
#endif

// warning messages
#ifdef LOG_NO_B2WARNING
#define B2WARNING(streamText)
#else
#define B2WARNING(streamText) { if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Warning,0,PACKAGENAME())) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Warning,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__)); } }
#endif

// error messages
#define B2ERROR(streamText) { std::ostringstream stringBuffer; stringBuffer << streamText; \
    LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Error,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__));}

// fatal messages
#define B2FATAL(streamText) { std::ostringstream stringBuffer; stringBuffer << streamText; \
    LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Fatal,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__));}

// scoped logging for entering/leaving methods
#ifdef LOG_NO_B2METHOD
#define B2METHOD()
#else
#define B2METHOD() LogMethod logMethod(PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__);
#endif

}  // end namespace Belle2

#endif  // LOGGER_H
