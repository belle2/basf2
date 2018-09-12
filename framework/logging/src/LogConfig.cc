/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConfig.h>
#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;


LogConfig::LogConfig(LogConfig::ELogLevel logLevel, int debugLevel) :
  m_logLevel(logLevel),
  m_debugLevel(debugLevel),
  m_abortLevel(LogConfig::c_Fatal)
{
  for (int level = 0; level < LogConfig::c_Default; level++) {
    m_logInfo[level] = 0;
  }
}


void LogConfig::setLogLevel(ELogLevel logLevel)
{
  if (logLevel == c_Fatal) {
    // we want to show a warning that this is not possible but the log level
    // might already be on error hiding the warning completely. We don't want
    // to emit an error because this might abort processing. So force a warning
    // message.
    Belle2::LogSystem::Instance().sendMessage(Belle2::LogMessage(c_Warning,
                                              "Error messages cannot be switched off. The log level is set to ERROR instead of FATAL.",
                                              PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__, 0));
    logLevel = c_Error;
  }
  m_logLevel = logLevel;
}


const char* LogConfig::logLevelToString(LogConfig::ELogLevel logLevelType)
{
  static const char* logLevelString[] = {"DEBUG", "INFO", "RESULT", "WARNING", "ERROR", "FATAL", "default"};
  return logLevelString[logLevelType];
}
