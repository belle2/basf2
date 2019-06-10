/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogMethod.h>
#include <framework/logging/LogConfig.h>
#include <framework/logging/LogMessage.h>
#include <framework/logging/LogSystem.h>

#include <utility>

using namespace std;
using namespace Belle2;

// cppcheck-suppress passedByValue ; We take a value to move it into a member so no performance penalty
LogMethod::LogMethod(const char* package, string  function, string  file, unsigned int line): m_package(package),
  m_function(std::move(function)), m_file(std::move(file)), m_line(line)
{
  //Send message for entering the method
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Info)) {
    string messageText = "--> Entering method '" + m_function + "'";
    LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Info, messageText, m_package, m_function, m_file, m_line));
  }
}

LogMethod::~LogMethod()
{
  //Check for uncaught exceptions
  string uncaughtExc;
  if (std::uncaught_exceptions() > 0) uncaughtExc = "(" + std::to_string(std::uncaught_exceptions()) +
                                                      " uncaught exceptions pending)";

  //Send message for leaving the method
  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Info)) {
    string messageText = "<-- Leaving method '" + m_function + "' " + uncaughtExc;
    LogSystem::Instance().sendMessage(LogMessage(LogConfig::c_Info, messageText, m_package, m_function, m_file, m_line));
  }
}
