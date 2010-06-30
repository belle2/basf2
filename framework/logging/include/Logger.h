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

#include <framework/logging/LogCommon.h>
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
#if defined(__GNUC__) || defined(__ICC) || defined(__ECC) || defined(__APPLE__)
#define FUNCTIONNAME() __FUNCTION__
#else
#define FUNCTIONNAME() "???"
#endif

// debug messages
#ifdef LOG_NO_DEBUG
#define DEBUG(level, streamText)
#define DEBUG_S(level, streamText)
#else
#define DEBUG(level, streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Debug,level)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Debug,streamText,PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__); } }

#define DEBUG_S(level, streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Debug,level)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Debug,streamText,PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__,false); } }
#endif

// info messages
#ifdef LOG_NO_INFO
#define INFO(streamText)
#define INFO_S(streamText)
#else
#define INFO(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Info)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Info,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__); } }

#define INFO_S(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Info)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Info,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__,false); } }
#endif

// warning messages
#ifdef LOG_NO_WARNING
#define WARNING(streamText)
#define WARNING_S(streamText)
#else
#define WARNING(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Warning)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Warning,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__); } }

#define WARNING_S(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Warning)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Warning,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__,false); } }
#endif

// error messages
#define ERROR(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Error)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Error,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__); } }

#define ERROR_S(streamText) { if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Error)) { \
      std::ostringstream stringBuffer; stringBuffer << streamText; \
      LogSystem::Instance().sendMessage(LogCommon::c_Error,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__,false); } }

// fatal messages
#define FATAL(streamText) { std::ostringstream stringBuffer; stringBuffer << streamText; \
    LogSystem::Instance().sendMessageForceAbort(LogCommon::c_Fatal,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__);}

#define FATAL_S(streamText) { std::ostringstream stringBuffer; stringBuffer << streamText; \
    LogSystem::Instance().sendMessageForceAbort(LogCommon::c_Fatal,stringBuffer.rdbuf()->str(),PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__,false);}

// scoped logging for entering/leaving methods
#ifdef LOG_NO_METHOD
#define LOG_METHOD()
#else
#define LOG_METHOD() LogMethod logMethod(PACKAGENAME(),FUNCTIONNAME(),__FILE__,__LINE__);
#endif

}  // end namespace Belle2

#endif  // LOGGER_H
