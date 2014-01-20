/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogMessage.h>

#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>
#include <framework/pcore/ProcHandler.h>

#include <ostream>

using namespace std;
using namespace Belle2;


LogMessage::LogMessage(LogConfig::ELogLevel logLevel, const std::string& message, const std::string& package,
                       const std::string& function, const std::string& file, unsigned int line) :
  m_logLevel(logLevel),
  m_message(message),
  m_module("-global-"),
  m_package(package),
  m_function(function),
  m_file(file),
  m_line(line),
  m_logInfo(0)
{
}


bool LogMessage::operator==(const LogMessage& message) const
{
  return (m_logLevel == message.m_logLevel) &&
         (m_message == message.m_message) &&
         (m_module == message.m_module) &&
         (m_package == message.m_package) &&
         (m_function == message.m_function) &&
         (m_file == message.m_file) &&
         (m_line == message.m_line);
}


std::ostream& LogMessage::print(std::ostream& out) const
{
  if (!m_logInfo || (m_logInfo & LogConfig::c_Timestamp)) {
    static const double startClock = Utils::getClock();
    out.precision(3);
    out << std::fixed << (Utils::getClock() - startClock) / Unit::s << ": ";
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Level)) {
    out << "[" << LogConfig::logLevelToString(m_logLevel) << "] ";
  }
  if (ProcHandler::EvtProcID() != -1) {
    //which process is this?
    out << "(" << ProcHandler::EvtProcID() << ") ";
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Message)) {
    out << m_message;
  }
  bool printLocation = (!m_logInfo || (m_logInfo & (LogConfig::c_Module + LogConfig::c_Package + LogConfig::c_Function + LogConfig::c_File + LogConfig::c_Line)));
  if (printLocation) {
    out << "  {";
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Module)) {
    out << " module: " << m_module;
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Package)) {
    out << " package: " << m_package;
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Function)) {
    out << " function: " << m_function;
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_File)) {
    out << " @" << m_file;
  }
  if (!m_logInfo || (m_logInfo & LogConfig::c_Line)) {
    out  << ":" << m_line;
  }
  if (printLocation) {
    out << " }";
  }
  out << endl;
  return out;
}


ostream& operator<< (ostream& out, const LogMessage& logMessage)
{
  return logMessage.print(out);
}

namespace Belle2 {
  size_t hash(const LogMessage& msg)
  {
    return (
             std::hash<std::string>()(msg.m_message)
             ^ std::hash<std::string>()(msg.m_module)
             ^ std::hash<std::string>()(msg.m_package)
             ^ std::hash<std::string>()(msg.m_function)
             ^ std::hash<std::string>()(msg.m_file)
             ^ msg.m_line
             ^ msg.m_logLevel
           );
  }
}
