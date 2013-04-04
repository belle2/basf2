/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogSystem.h>
#include <framework/logging/LogMessage.h>
#include <framework/logging/LogConnectionBase.h>
#include <framework/logging/LogConnectionIOStream.h>
#include <framework/logging/Logger.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Belle2;
using namespace std;


LogSystem& LogSystem::Instance()
{
  static LogSystem instance;
  return instance;
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


bool LogSystem::isLevelEnabled(LogConfig::ELogLevel level, int debugLevel, const std::string& package) const
{
  LogConfig::ELogLevel logLevelLimit = LogConfig::c_Default;
  int debugLevelLimit = 0;

  // first check whether the log level of the current package is set
  map<string, LogConfig>::const_iterator packageLogConfig = m_packageLogConfigs.find(package);
  if (packageLogConfig != m_packageLogConfigs.end()) {
    const LogConfig& logConfig = packageLogConfig->second;
    if (logConfig.getLogLevel() != LogConfig::c_Default) {
      logLevelLimit = logConfig.getLogLevel();
      debugLevelLimit = logConfig.getDebugLevel();
    }
  }

  // if there's no package log level set, take it either from the module or the global settings
  if (logLevelLimit == LogConfig::c_Default) {
    if (m_moduleLogConfig && (m_moduleLogConfig->getLogLevel() != LogConfig::c_Default)) {
      logLevelLimit = m_moduleLogConfig->getLogLevel();
      debugLevelLimit = m_moduleLogConfig->getDebugLevel();
    } else {
      logLevelLimit = m_logConfig.getLogLevel();
      debugLevelLimit = m_logConfig.getDebugLevel();
    }
  }

  return ((level != LogConfig::c_Debug) && (logLevelLimit <= level)) ||
         ((level == LogConfig::c_Debug) && (logLevelLimit == LogConfig::c_Debug) && (debugLevelLimit >= debugLevel));
}


bool LogSystem::sendMessage(LogMessage message)
{
  LogConfig::ELogLevel logLevel = message.getLogLevel();
  map<string, LogConfig>::iterator packageLogConfig = m_packageLogConfigs.find(message.getPackage());
  if ((packageLogConfig != m_packageLogConfigs.end()) && packageLogConfig->second.getLogInfo(logLevel)) {
    message.setLogInfo(packageLogConfig->second.getLogInfo(logLevel));
  } else if (m_moduleLogConfig && m_moduleLogConfig->getLogInfo(logLevel)) {
    message.setLogInfo(m_moduleLogConfig->getLogInfo(logLevel));
  } else {
    message.setLogInfo(m_logConfig.getLogInfo(logLevel));
  }

  message.setModule(m_moduleName);

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
    printErrorSummary();
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


int LogSystem::getMessageCounter(LogConfig::ELogLevel logLevel) const
{
  return m_messageCounter[logLevel];
}


LogConfig::ELogLevel LogSystem::getCurrentLogLevel() const
{
  //Check if module specific logging is set
  if (m_moduleLogConfig && (m_moduleLogConfig->getLogLevel() != LogConfig::c_Default)) {
    return m_moduleLogConfig->getLogLevel();
  }
  return m_logConfig.getLogLevel();
}


//============================================================================
//                              Private methods
//============================================================================

LogSystem::LogSystem() :
  m_logConfig(LogConfig::c_Info, 100),
  m_moduleLogConfig(0)
{
  unsigned int logInfo = LogConfig::c_Level + LogConfig::c_Message;
  unsigned int warnLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module;
  unsigned int debugLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_File + LogConfig::c_Line;
  unsigned int fatalLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_Function;
  m_logConfig.setLogInfo(LogConfig::c_Debug, debugLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Info, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Warning, warnLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Error, warnLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Fatal, fatalLogInfo);

  resetMessageCounter();

  //enable color for TTYs with color support (list taken from gtest)
  const bool isTTY = isatty(fileno(stdout));
  const string termName = getenv("TERM") ? getenv("TERM") : "";
  const bool useColor = isTTY and(
                          termName == "xterm" or
                          termName == "xterm-color" or
                          termName == "xterm-256color" or
                          termName == "linux" or
                          termName == "cygwin");

  addLogConnection(new LogConnectionIOStream(std::cout, useColor));
}

void LogSystem::printErrorSummary()
{
  int numLogWarn = getMessageCounter(LogConfig::c_Warning);
  int numLogError = getMessageCounter(LogConfig::c_Error);
  LogConfig oldConfig = m_logConfig;
  // only show level & message
  m_logConfig.setLogInfo(LogConfig::c_Warning, LogConfig::c_Level | LogConfig::c_Message);
  m_logConfig.setLogInfo(LogConfig::c_Error, LogConfig::c_Level | LogConfig::c_Message);
  if (numLogWarn)
    B2WARNING(numLogWarn << " warnings occured.");
  if (numLogError)
    B2ERROR(numLogError << " errors occured.");
  m_logConfig = oldConfig;
}

LogSystem::~LogSystem()
{
  resetLogConnections();
}


void LogSystem::incMessageCounter(LogConfig::ELogLevel logLevel)
{
  m_messageCounter[logLevel]++;
}
