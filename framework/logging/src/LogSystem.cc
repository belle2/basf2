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

#include <iostream>
#include <unordered_map>
#include <functional>

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
  const LogConfig& config = getCurrentLogConfig(package);
  LogConfig::ELogLevel logLevelLimit = config.getLogLevel();
  int debugLevelLimit = config.getDebugLevel();

  return logLevelLimit <= level && (logLevelLimit != LogConfig::c_Debug || debugLevelLimit >= debugLevel);
}


bool LogSystem::sendMessage(LogMessage message)
{
  LogConfig::ELogLevel logLevel = message.getLogLevel();
  map<string, LogConfig>::const_iterator packageLogConfig = m_packageLogConfigs.find(message.getPackage());
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
  if (m_printErrorSummary && logLevel >= LogConfig::c_Warning) {
    m_errorLog.push_back(message);
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


const LogConfig& LogSystem::getCurrentLogConfig(const std::string& package) const
{
  //package specific config?
  if (!package.empty()) {
    map<string, LogConfig>::const_iterator packageLogConfig = m_packageLogConfigs.find(package);
    if (packageLogConfig != m_packageLogConfigs.end()) {
      const LogConfig& logConfig = packageLogConfig->second;
      if (logConfig.getLogLevel() != LogConfig::c_Default)
        return logConfig;
    }
  }

  //module specific config?
  if (m_moduleLogConfig && (m_moduleLogConfig->getLogLevel() != LogConfig::c_Default)) {
    return *m_moduleLogConfig;
  }

  //global config
  return m_logConfig;
}


//============================================================================
//                              Private methods
//============================================================================

LogSystem::LogSystem() :
  m_logConfig(LogConfig::c_Info, 100),
  m_moduleLogConfig(0),
  m_printErrorSummary(false)
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
  const bool useColor = isTTY and (
                          termName == "xterm" or
                          termName == "xterm-color" or
                          termName == "xterm-256color" or
                          termName == "linux" or
                          termName == "cygwin");

  addLogConnection(new LogConnectionIOStream(std::cout, useColor));

  m_errorLog.reserve(100);
}

void LogSystem::printErrorSummary()
{
  if (!m_printErrorSummary)
    return; //disabled

  int numLogWarn = getMessageCounter(LogConfig::c_Warning);
  int numLogError = getMessageCounter(LogConfig::c_Error);
  if (m_errorLog.empty())
    return; //nothing to do

  const LogConfig oldConfig = m_logConfig;
  m_logConfig.setAbortLevel(LogConfig::c_Default); //prevent calling printErrorSummary() again when printing

  // only show level & message
  m_logConfig.setLogInfo(LogConfig::c_Warning, LogConfig::c_Level | LogConfig::c_Message);
  m_logConfig.setLogInfo(LogConfig::c_Error, LogConfig::c_Level | LogConfig::c_Message);
  m_logConfig.setLogLevel(LogConfig::c_Info);

  B2INFO("================================================================================");
  B2INFO("Error summary: " << numLogError << " errors and " << numLogWarn << " warnings occurred.");


  //start with 100 entries in hash map
  std::function<size_t (const LogMessage&)> hashFunction = &hash;
  std::unordered_map<LogMessage, int, decltype(hashFunction)> errorCount(100, hashFunction);

  //log in chronological order, with repetitions removed
  std::vector<LogMessage> uniqueLog;
  uniqueLog.reserve(100);

  for (const LogMessage & msg : m_errorLog) {
    int count = errorCount[msg]++;

    if (count == 0) //this is the first time we see this message
      uniqueLog.push_back(msg);
  }
  m_errorLog.clear();

  for (const LogMessage & msg : uniqueLog) {
    sendMessage(msg);

    int count = errorCount[msg];
    if (count != 1) {
      B2INFO(" (last message occurred " << count << " times in total)");
    }
  }
  B2INFO("================================================================================\n");

  m_printErrorSummary = false; // only do this once (e.g. not again when used through python)
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
