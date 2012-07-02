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
    B2WARNING("Error messages cannot be switched off. The log level is set to ERROR instead of FATAL.");
    logLevel = c_Error;
  }
  m_logLevel = logLevel;
}


const char* LogConfig::logLevelToString(LogConfig::ELogLevel logLevelType)
{
  static const char* logLevelString[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL", "default"};
  return logLevelString[logLevelType];
}
