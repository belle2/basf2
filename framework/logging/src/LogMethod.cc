/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <logging/LogMethod.h>

using namespace std;
using namespace Belle2;


LogMethod::LogMethod(const string& package, const string& function, const string& file, unsigned int line)
{
  m_package = package;
  m_function = function;
  m_file = file;
  m_line = line;

  //Send message for entering the method
  if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Info)) {
    string messageText = "--> Entering method '" + m_function + "'";
    LogSystem::Instance().sendMessage(LogCommon::c_Info, messageText, m_package, m_function, m_file, m_line, true);
  }
}


LogMethod::~LogMethod()
{
  //Check for uncaught exceptions
  string uncaughtExc;
  if (std::uncaught_exception()) uncaughtExc = "(uncaught exceptions pending)";

  //Send message for leaving the method
  if (LogSystem::Instance().isLevelEnabled(LogCommon::c_Info)) {
    string messageText = "<-- Leaving method '" + m_function + "' " + uncaughtExc;
    LogSystem::Instance().sendMessage(LogCommon::c_Info, messageText, m_package, m_function, m_file, m_line, true);
  }
}
