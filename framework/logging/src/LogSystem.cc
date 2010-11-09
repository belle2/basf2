/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogSystem.h>
#include <framework/logging/LogConnectionIOStream.h>

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


void LogSystem::addLogConnection(LogConnectionBase* logConnection)
{
  m_logConnections.push_back(logConnection);
}


void LogSystem::resetLogConnections()
{
  for (unsigned int i = 0; i < m_logConnections.size(); i++) {
    delete m_logConnections[i];
  }
  m_logConnections.clear();
}


bool LogSystem::isLevelEnabled(LogConfig::ELogLevel level, int debugLevel, std::string package)
{
  LogConfig::ELogLevel logLevelLimit = m_logConfig.getLogLevel();
  int debugLevelLimit = m_logConfig.getDebugLevel();

  map<string, LogConfig>::iterator packageLogConfig = m_packageLogConfigs.find(package);
  if (packageLogConfig != m_packageLogConfigs.end()) {
    LogConfig& logConfig = packageLogConfig->second;
    logLevelLimit = logConfig.getLogLevel();
    debugLevelLimit = logConfig.getDebugLevel();
  } else if (m_moduleLogConfig && (m_moduleLogConfig->getLogLevel() != LogConfig::c_Default)) {
    logLevelLimit = m_moduleLogConfig->getLogLevel();
    debugLevelLimit = m_moduleLogConfig->getDebugLevel();
  }

  return ((level != LogConfig::c_Debug) && (logLevelLimit <= level)) ||
         ((level == LogConfig::c_Debug) && (logLevelLimit == LogConfig::c_Debug) && (debugLevelLimit >= debugLevel));
}


bool LogSystem::sendMessage(LogMessage message)
{
  LogConfig::ELogLevel logLevel = message.getLogLevel();
  map<string, LogConfig>::iterator packageLogConfig = m_packageLogConfigs.find(message.getPackage());
  if (packageLogConfig != m_packageLogConfigs.end()) {
    message.setLogInfo(packageLogConfig->second.getLogInfo(logLevel));
  } else if (m_moduleLogConfig && m_moduleLogConfig->getLogInfo(logLevel)) {
    message.setLogInfo(m_moduleLogConfig->getLogInfo(logLevel));
  } else {
    message.setLogInfo(m_logConfig.getLogInfo(logLevel));
  }

  bool messageSent = false;
  for (unsigned int i = 0; i < m_logConnections.size(); i++) {
    if (m_logConnections[i]->sendMessage(message)) {
      messageSent = true;
    }
  }

  if (messageSent) {
    incMessageCounter(logLevel);
  }

  if (logLevel >= m_logConfig.getAbortLevel()) {
    abort();
  }

  return messageSent;
}


void LogSystem::resetMessageCounter()
{
  for (int i = 0; i < LogConfig::c_Default; ++i) {
    m_messageCounter[i] = 0;
  }
}


int LogSystem::getMessageCounter(LogConfig::ELogLevel logLevel)
{
  return m_messageCounter[logLevel];
}


//============================================================================
//                              Private methods
//============================================================================

LogSystem::LogSystem() :
    m_logConfig(LogConfig::c_Info, 100),
    m_moduleLogConfig(0)
{
  unsigned int logInfo = LogConfig::c_Level + LogConfig::c_Message;
  unsigned int debugLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_File + LogConfig::c_Line;
  unsigned int fatalLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_Function;
  m_logConfig.setLogInfo(LogConfig::c_Debug, debugLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Info, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Warning, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Error, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Fatal, fatalLogInfo);

//  m_messageCounter = new int[LogConfig::c_Default];
  resetMessageCounter();

  addLogConnection(new LogConnectionIOStream(std::cout));
}


LogSystem::~LogSystem()
{
  resetLogConnections();
}


void LogSystem::incMessageCounter(LogConfig::ELogLevel logLevel)
{
  m_messageCounter[logLevel]++;
}
