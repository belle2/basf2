/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogMessage.h>

#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ProcessStatistics.h>

#include <boost/property_tree/json_parser.hpp>
#include <ostream>
#include <utility>

using namespace std;
using namespace Belle2;

LogMessage::LogMessage(LogConfig::ELogLevel logLevel, const std::string& message, const char* package,
                       std::string  function, std::string  file, unsigned int line, int debugLevel) :
  m_logLevel(logLevel),
  m_message(message),
  m_module(""),
  m_package(package ? package : ""),
  m_function(std::move(function)),
  m_file(std::move(file)),
  m_line(line),
  m_debugLevel(debugLevel),
  m_logInfo(0),
  m_count(0)
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
  m_logInfo(0),
  m_count(0)
{
  messageStream.adjustLogLevel(m_logLevel);
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
    if ((vars.size() > 0 or complete) and !(logInfo & LogConfig::c_NoVariables)) {
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
  if (complete) {
    buffer << ",\"count\":" << m_count;
    StoreObjPtr<EventMetaData> eventMetaData;
    if (eventMetaData.isValid()) {
      buffer << ",\"experiment\":" << eventMetaData->getExperiment();
      buffer << ",\"run\":" << eventMetaData->getRun();
      buffer << ",\"subrun\":" << eventMetaData->getSubrun();
      buffer << ",\"event\":" << eventMetaData->getEvent();
    }
    StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
    if (processStatistics.isValid()) {
      const auto& stats = processStatistics->getGlobal();
      buffer << ",\"nruns\":" << int(stats.getCalls(ModuleStatistics::EStatisticCounters::c_BeginRun));
      buffer << ",\"nevents\":" << int(stats.getCalls());
    }
    buffer << ",\"initialize\":" << ((DataStore::Instance().getInitializeActive()) ? "true" : "false");
  }
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
    out << m_message.str(!(logInfo & LogConfig::c_NoVariables));
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
