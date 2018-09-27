/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionFilter.h>

#include <sstream>

using namespace Belle2;
using namespace std;

LogConnectionFilter::LogConnectionFilter(LogConnectionBase* logConnection) :
  m_logConnection(logConnection),
  m_previousMessage(LogConfig::c_Default, "", "", "", "", 0),
  m_repeatCounter(0)
{
}


LogConnectionFilter::~LogConnectionFilter()
{
  printRepetitions();
}


bool LogConnectionFilter::isConnected()
{
  return (m_logConnection && m_logConnection->isConnected());
}


bool LogConnectionFilter::sendMessage(const LogMessage& message)
{

  if (message == m_previousMessage) {
    m_repeatCounter++;
    // return true anyway since we will print the repeat message later so we
    // consider this successful
    return true;
  }

  printRepetitions();
  m_previousMessage = message;

  return m_logConnection->sendMessage(message);
}


void LogConnectionFilter::printRepetitions()
{
  if (m_repeatCounter > 0) {
    const std::string message{" ... message repeated " + std::to_string(m_repeatCounter) + " times"};
    LogMessage repeatMessage(m_previousMessage.getLogLevel(), message, "", "", "", 0);
    repeatMessage.setLogInfo(LogConfig::c_Message);
    m_logConnection->sendMessage(repeatMessage);
  }
  m_repeatCounter = 0;
}
