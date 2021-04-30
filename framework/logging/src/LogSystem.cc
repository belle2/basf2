/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogSystem.h>
#include <framework/logging/LogMessage.h>
#include <framework/logging/LogConnectionBase.h>
#include <framework/logging/LogConnectionFilter.h>
#include <framework/logging/LogConnectionConsole.h>
#include <framework/datastore/DataStore.h>

#include <TROOT.h>
#include <csignal>

#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <functional>

using namespace Belle2;
using namespace std;


bool LogSystem::s_debugEnabled = false;


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
  for (auto connection : m_logConnections) {
    delete connection;
  }
  m_logConnections.clear();
}


bool LogSystem::deliverMessageToConnections(const LogMessage& message)
{
  bool messageSent = false;
  for (auto con : m_logConnections) {
    messageSent |= con->sendMessage(message);
  }
  return messageSent;
}

void LogSystem::showText(LogConfig::ELogLevel level, const std::string& txt, int info)
{
  LogMessage customText(level, txt, "", "", "", 0);
  customText.setLogInfo(info);
  deliverMessageToConnections(customText);
}

bool LogSystem::sendMessage(LogMessage&& message)
{
  LogConfig::ELogLevel logLevel = message.getLogLevel();
  auto packageLogConfig = m_packageLogConfigs.find(message.getPackage());
  if ((packageLogConfig != m_packageLogConfigs.end()) && packageLogConfig->second.getLogInfo(logLevel)) {
    message.setLogInfo(packageLogConfig->second.getLogInfo(logLevel));
  } else if (m_moduleLogConfig && m_moduleLogConfig->getLogInfo(logLevel)) {
    message.setLogInfo(m_moduleLogConfig->getLogInfo(logLevel));
  } else {
    message.setLogInfo(m_logConfig.getLogInfo(logLevel));
  }

  message.setModule(m_moduleName);

  // We want to count it whether we've seen it or not
  incMessageCounter(logLevel);

  // add message to list of message or increase repetition value
  bool lastTime(false);
  unsigned int repetition{0};
  if (m_messageLog.size() >= c_errorSummaryMaxLines) {
    // we already have maximum size of the error log so don't add more messages.
    // but we might want to increase the counter if it already exists
    /* cppcheck-suppress stlIfFind */
    if (auto it = m_messageLog.find(message); it != m_messageLog.end()) {
      repetition = ++(it->second);
    }
  } else if (logLevel >= LogConfig::c_Warning or m_maxErrorRepetition > 0) {
    // otherwise we only keep warnings or higher unless we suppress repetitions,
    // then we keep everything
    repetition = ++m_messageLog[message];
    message.setCount(repetition);
  }
  lastTime = m_maxErrorRepetition > 0 and repetition == m_maxErrorRepetition;
  if (m_maxErrorRepetition > 0 and repetition > m_maxErrorRepetition) {
    // We've seen this message more than once before so it cannot be an abort
    // level message. So we can just not do anything here except counting ...
    ++m_suppressedMessages;
    // However we should warn from time to time that messages are being
    // suppressed ... but not too often otherwise we don't gain anything so
    // let's warn first each 100, then each 1000 suppressed messages.
    if ((m_suppressedMessages < 1000 and m_suppressedMessages % 100 == 0) or
        (m_suppressedMessages < 10000 and m_suppressedMessages % 1000 == 0) or
        (m_suppressedMessages % 10000 == 0)) {
      showText(LogConfig::c_Warning, "... " + std::to_string(m_suppressedMessages) + " log messages suppressed due to repetition ...");
    }
    return true;
  }

  // Ok we want to see the message
  bool messageSent = deliverMessageToConnections(message);

  if (logLevel >= m_logConfig.getAbortLevel()) {
    printErrorSummary();
    // make sure loc connections are finalized to not loose output
    for (auto connection : m_logConnections) {
      connection->finalizeOnAbort();
    }
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers

    //in good tradition, ROOT signal handlers are unsafe.
    //this avoids a problem seen in ROOT's atexit() handler, which might crash
    //and then deadlock the process in the SIGSEGV handler...
    signal(SIGSEGV, nullptr);

    //ROOT will also try to do this, but doesn't get the order right.
    gROOT->CloseFiles();
    exit(1);
  }

  // And if it is the last time we show it let's add a trailer for good measure
  if (messageSent and lastTime) {
    showText(max(message.getLogLevel(), LogConfig::c_Warning),
             "The previous message has occurred " + std::to_string(m_maxErrorRepetition) +
             " times and will be suppressed in future",
             LogConfig::c_Level | LogConfig::c_Message);
  }

  return messageSent;
}


void LogSystem::resetMessageCounter()
{
  for (int& i : m_messageCounter) {
    i = 0;
  }
  m_suppressedMessages = 0;
  m_messageLog.clear();
  m_messageLog.reserve(100);
}


int LogSystem::getMessageCounter(LogConfig::ELogLevel logLevel) const
{
  return m_messageCounter[logLevel];
}


//============================================================================
//                              Private methods
//============================================================================

LogSystem::LogSystem() :
  m_logConfig(LogConfig::c_Info),
  m_moduleLogConfig(nullptr),
  m_printErrorSummary(false),
  m_messageCounter{0}
{
  resetLogging();
}

void LogSystem::resetLogging()
{
  m_logConfig.setLogLevel(LogConfig::c_Info);
  m_logConfig.setDebugLevel(LogConfig::c_DefaultDebugLevel);
  m_moduleLogConfig = nullptr;
  m_packageLogConfigs.clear();
  constexpr unsigned int logInfo = LogConfig::c_Level + LogConfig::c_Message;
  constexpr unsigned int warnLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module;
  constexpr unsigned int debugLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_File +
                                        LogConfig::c_Line;
  constexpr unsigned int fatalLogInfo = LogConfig::c_Level + LogConfig::c_Message + LogConfig::c_Module + LogConfig::c_Function;
  m_logConfig.setLogInfo(LogConfig::c_Debug, debugLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Info, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Result, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Warning, warnLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Error, warnLogInfo);
  m_logConfig.setLogInfo(LogConfig::c_Fatal, fatalLogInfo);

  resetMessageCounter();
  resetLogConnections();
  addLogConnection(new LogConnectionFilter(new LogConnectionConsole(STDOUT_FILENO)));

  m_printErrorSummary = false;
  s_debugEnabled = false;
}

void LogSystem::printErrorSummary()
{
  if (!m_printErrorSummary)
    return; //disabled

  int numLogWarn = getMessageCounter(LogConfig::c_Warning);
  int numLogError = getMessageCounter(LogConfig::c_Error);

  // ok, only errors and warnings in the error summary
  std::vector<std::pair<LogMessage, unsigned int>> messages;
  for (auto && value : m_messageLog) {
    if (value.first.getLogLevel() < LogConfig::c_Warning) continue;
    messages.emplace_back(std::move(value));
  }
  // but show them sorted by severity and occurrence
  std::stable_sort(messages.begin(), messages.end(), [](const auto & a, const auto & b) {
    return a.first.getLogLevel() > b.first.getLogLevel() or
           (a.first.getLogLevel() == b.first.getLogLevel() and  a.second > b.second);
  });
  if (messages.size() == 0) {
    return; //nothing to do
  }

  // save configuration
  const LogConfig oldConfig = m_logConfig;
  // and make sure module configuration is bypassed, otherwise changing the settings in m_logConfig would be ignored
  const LogConfig* oldModuleConfig {nullptr};
  std::swap(m_moduleLogConfig, oldModuleConfig);
  // similar for package configuration
  map<string, LogConfig> oldPackageConfig;
  std::swap(m_packageLogConfigs, oldPackageConfig);
  // prevent calling printErrorSummary() again when printing
  m_logConfig.setAbortLevel(LogConfig::c_Default);

  // only show level & message
  int logInfo = LogConfig::c_Level | LogConfig::c_Message;
  m_logConfig.setLogInfo(LogConfig::c_Info, logInfo);
  logInfo |= LogConfig::c_Module;
  m_logConfig.setLogInfo(LogConfig::c_Warning, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Error, logInfo);
  m_logConfig.setLogInfo(LogConfig::c_Fatal, logInfo);
  m_logConfig.setLogLevel(LogConfig::c_Info);

  // and then show all the messages
  showText(LogConfig::c_Info, "===Error Summary================================================================",
           LogConfig::c_Message | LogConfig::c_Level);
  for (auto & [msg, count] : messages) {
    const bool multiple = count > 1;
    // don't show variables if the message appeared more than once, could have different values/variables
    msg.setLogInfo(m_logConfig.getLogInfo(msg.getLogLevel()) | (multiple ? LogConfig::c_NoVariables : 0));
    // and directly print the message as it was saved.
    deliverMessageToConnections(msg);
    // and also tell us the count
    if (multiple) {
      showText(LogConfig::c_Info, "    (last message occurred " + std::to_string(count) + " times)");
    }
  }
  showText(LogConfig::c_Info, "================================================================================",
           LogConfig::c_Message | LogConfig::c_Level);
  if (numLogError) {
    showText(LogConfig::c_Error, "in total, " + std::to_string(numLogError) + " errors occurred during processing",
             LogConfig::c_Message | LogConfig::c_Level);
  }
  if (numLogWarn) {
    showText(LogConfig::c_Warning, "in total, " + std::to_string(numLogWarn) + " warnings occurred during processing",
             LogConfig::c_Message | LogConfig::c_Level);
  }
  if (m_messageLog.size() == c_errorSummaryMaxLines) {
    showText(LogConfig::c_Warning, "Note: The error summary was truncated to " +
             std::to_string(c_errorSummaryMaxLines) + " (distinct) messages",
             LogConfig::c_Message | LogConfig::c_Level);
  }
  if (m_suppressedMessages) {
    showText(LogConfig::c_Warning, std::to_string(m_suppressedMessages) + " log messages were suppressed",
             LogConfig::c_Message | LogConfig::c_Level);
  }

  // and then clear the log
  m_messageLog.clear();

  // restore old configuration
  m_logConfig = oldConfig;
  std::swap(m_moduleLogConfig, oldModuleConfig);
  std::swap(m_packageLogConfigs, oldPackageConfig);
}

LogSystem::~LogSystem()
{
  resetLogConnections();
}

void LogSystem::incMessageCounter(LogConfig::ELogLevel logLevel)
{
  m_messageCounter[logLevel]++;
}
