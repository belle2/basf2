/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogSystem.h>

#include <stdio.h>
#include <stdlib.h>

using namespace Belle2;
using namespace std;

LogSystem* LogSystem::m_instance = NULL;


LogSystem& LogSystem::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new LogSystem();
  return *m_instance;
}


void LogSystem::setLogConnection(LogConnectionAbs* logConnection)
{
  if (m_logConnection != NULL) delete m_logConnection;
  m_logConnection = logConnection;
}


bool LogSystem::isLevelEnabled(LogCommon::ELogLevel level, int debugLevel)
{
  return ((level != LogCommon::c_Debug) && (m_logLevel <= level)) ||
         ((level == LogCommon::c_Debug) && (m_logLevel == LogCommon::c_Debug) && (m_debugLevel >= debugLevel));
}


bool LogSystem::sendMessage(LogCommon::ELogLevel logLevel, const string& message, const string& package,
                            const string& function, const string& file, unsigned int line, bool sendLocationInfo)
{
  if (m_logConnection == NULL) return false;

  bool messageSent = m_logConnection->sendMessage(logLevel, message, package, function, file, line, sendLocationInfo);

  if (messageSent) {
    incMessageCounter(logLevel);
  }
  return messageSent;
}


void LogSystem::sendMessageForceAbort(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                                      const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo)
{
  if (m_logConnection != NULL) {
    m_logConnection->sendMessage(logLevel, message, package, function, file, line, sendLocationInfo);
  }
  abort();
}


void LogSystem::resetMessageCounter()
{
  for (int i = 0; i < LogCommon::ELogLevelCount; ++i) {
    m_messageCounter[i] = 0;
  }
}


int LogSystem::getMessageCounter(LogCommon::ELogLevel logLevel)
{
  return m_messageCounter[logLevel];
}


//============================================================================
//                              Private methods
//============================================================================

LogSystem::LogSystem()
{
  m_logConnection = NULL;
  m_debugLevel = 100;
  m_logLevel = LogCommon::c_Info;

  m_messageCounter = new int[LogCommon::ELogLevelCount];
  resetMessageCounter();

  setLogConnection(new LogConnectionIOStream(std::cout));
}


LogSystem::~LogSystem()
{
  delete [] m_messageCounter;
  if (m_logConnection != NULL) delete m_logConnection;
}


void LogSystem::incMessageCounter(LogCommon::ELogLevel logLevel)
{
  m_messageCounter[logLevel]++;
}
