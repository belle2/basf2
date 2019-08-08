/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr, Thomas Hauth                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogMessage.h>

#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>
#include <framework/pcore/ProcHandler.h>

#include <boost/property_tree/json_parser.hpp>
#include <ostream>
#include <utility>

using namespace std;
using namespace Belle2;

LogMessage::LogMessage(LogConfig::ELogLevel logLevel, const std::string& message, const char* package,
// cppcheck-suppress passedByValue ; We take a value to move it into a member so no performance penalty
                       std::string  function, std::string  file, unsigned int line, int debugLevel) :
  m_logLevel(logLevel),
  m_message(message),
  m_module(""),
  m_package(package ? package : ""),
  m_function(std::move(function)),
  m_file(std::move(file)),
  m_line(line),
  m_debugLevel(debugLevel),
  m_logInfo(0)
{
}

LogMessage::LogMessage(LogConfig::ELogLevel logLevel, LogVariableStream&& messageStream, const char* package,
                       std::string  function, std::string  file, unsigned int line, int debugLevel) :
  m_logLevel(logLevel),
  m_message(std::move(messageStream)),
  m_module(""),
  m_package(package ? package : ""),
  m_function(std::move(function)),
  m_file(std::move(file)),
  m_line(line),
  m_debugLevel(debugLevel),
  m_logInfo(0)
{
}


bool LogMessage::operator==(const LogMessage& message) const
{
  return ((m_logLevel == message.m_logLevel) &&
          (m_line == message.m_line) &&
          (m_message == message.m_message) &&
          (m_module == message.m_module) &&
          (m_package == message.m_package) &&
          (m_function == message.m_function) &&
          (m_file == message.m_file));
}

std::string LogMessage::toJSON(bool complete) const
{
  using namespace boost::property_tree::json_parser;
  std::stringstream buffer;
  static const double startClock = Utils::getClock();
  double time = (Utils::getClock() - startClock) / Unit::s;
  int logInfo = (m_logInfo and not complete) ? m_logInfo :
                (LogConfig::c_Timestamp | LogConfig::c_Level | LogConfig::c_Message | LogConfig::c_Module |
                 LogConfig::c_Package | LogConfig::c_Function | LogConfig::c_File | LogConfig::c_Line);
  // in JSON we always output level, independent of what the log info says, otherwise it is hard to parse
  buffer << R"({"level":")" << LogConfig::logLevelToString(m_logLevel) << '"';
  if (logInfo & LogConfig::c_Message) {
    buffer << R"(,"message":")" << create_escapes(m_message.getMessage()) << '"';
    const auto& vars = m_message.getVariables();
    if (vars.size() > 0 or complete) {
      buffer << ",\"variables\":{";
      bool first{true};
      for (const auto& v : vars) {
        if (!first) buffer << ",";
        buffer << '"' << create_escapes(v.getName()) << "\":\"" << create_escapes(v.getValue()) << '"';
        first = false;
      }
      buffer << '}';
    }
  }
  if (logInfo & LogConfig::c_Module)
    buffer << R"(,"module":")" << create_escapes(m_module) << '"';
  if (logInfo & LogConfig::c_Package)
    buffer << R"(,"package":")" << create_escapes(m_package) << '"';
  if (logInfo & LogConfig::c_Function)
    buffer << R"(,"function":")" << create_escapes(m_function) << '"';
  if (logInfo & LogConfig::c_File)
    buffer << R"(,"file":")" << create_escapes(m_file) << '"';
  if (logInfo & LogConfig::c_Line)
    buffer << ",\"line\":" << m_line;
  if (logInfo & LogConfig::c_Timestamp)
    buffer << ",\"timestamp\":" << std::fixed << std::setprecision(3) << time;
  if (ProcHandler::EvtProcID() != -1 or complete)
    buffer << ",\"proc\":" << ProcHandler::EvtProcID();
  //variables ...
  buffer << "}\n";
  return buffer.str();
}

std::ostream& LogMessage::print(std::ostream& out) const
{
  int logInfo = m_logInfo ? m_logInfo :
                (LogConfig::c_Timestamp | LogConfig::c_Level | LogConfig::c_Message | LogConfig::c_Module | LogConfig::c_Package |
                 LogConfig::c_Function | LogConfig::c_File | LogConfig::c_Line);
  if (logInfo & LogConfig::c_Timestamp) {
    static const double startClock = Utils::getClock();
    const auto flags = out.flags();
    const int oldprecision = out.precision(3);
    out << std::fixed << (Utils::getClock() - startClock) / Unit::s << ": ";
    out.precision(oldprecision);
    out.flags(flags);
  }
  if (logInfo & LogConfig::c_Level) {
    const std::string debugLevel = (m_logLevel == LogConfig::c_Debug) ? (":" + std::to_string(m_debugLevel)) : "";
    out << "[" << LogConfig::logLevelToString(m_logLevel) << debugLevel << "] ";
  }
  if (ProcHandler::EvtProcID() != -1) {
    //which process is this?
    out << "(" << ProcHandler::EvtProcID() << ") ";
  }
  if (logInfo & LogConfig::c_Message) {
    out << m_message.str();
  }
  // if there is no module or package or similar there's no need to print them
  if (m_module.empty()) logInfo &= ~LogConfig::c_Module;
  if (m_package.empty()) logInfo &= ~LogConfig::c_Package;
  if (m_function.empty()) logInfo &= ~LogConfig::c_Function;
  // line number without filename is useless as well so disable both in one go
  if (m_file.empty()) logInfo &= ~(LogConfig::c_File | LogConfig::c_Line);
  // is there any location string left to print?
  bool printLocation = logInfo & (LogConfig::c_Module | LogConfig::c_Package | LogConfig::c_Function |
                                  LogConfig::c_File | LogConfig::c_Line);
  if (printLocation) {
    out << "  {";
  }
  if (logInfo & LogConfig::c_Module) {
    out << " module: " << m_module;
  }
  if (logInfo & LogConfig::c_Package) {
    out << " package: " << m_package;
  }
  if (logInfo & LogConfig::c_Function) {
    out << " function: " << m_function;
  }
  if (logInfo & LogConfig::c_File) {
    out << " @" << m_file;
  }
  if (logInfo & LogConfig::c_Line) {
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
             std::hash<std::string>()(msg.m_message.str())
             ^ std::hash<std::string>()(msg.m_module)
             ^ std::hash<std::string>()(msg.m_package)
             ^ std::hash<std::string>()(msg.m_function)
             ^ std::hash<std::string>()(msg.m_file)
             ^ msg.m_line
             ^ msg.m_logLevel
           );
  }
}
