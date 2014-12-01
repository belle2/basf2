/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Thomas Kuhr                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/pybasf2/LogPythonInterface.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogConnectionFilter.h>
#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogConnectionIOStream.h>

#include <framework/core/Environment.h>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace boost::python;


void LogPythonInterface::setLogLevel(LogConfig::ELogLevel level)
{
  LogConfig::ELogLevel overrideLevel = (LogConfig::ELogLevel)Environment::Instance().getLogLevelOverride();
  if (overrideLevel != LogConfig::c_Default)
    level = overrideLevel;

  LogSystem::Instance().getLogConfig()->setLogLevel(level);
}

void LogPythonInterface::setAbortLevel(LogConfig::ELogLevel level)
{
  LogSystem::Instance().getLogConfig()->setAbortLevel(level);
}

void LogPythonInterface::setDebugLevel(int level)
{
  LogSystem::Instance().getLogConfig()->setDebugLevel(level);
}

void LogPythonInterface::setLogInfo(LogConfig::ELogLevel level, int info)
{
  LogSystem::Instance().getLogConfig()->setLogInfo(level, info);
}

void LogPythonInterface::setPackageLogConfig(const std::string& package, const LogConfig& config)
{
  LogSystem::Instance().addPackageLogConfig(package, config);
}

LogConfig::ELogLevel LogPythonInterface::getLogLevel()
{
  return LogSystem::Instance().getLogConfig()->getLogLevel();
}

LogConfig::ELogLevel LogPythonInterface::getAbortLevel()
{
  return LogSystem::Instance().getLogConfig()->getAbortLevel();
}

int LogPythonInterface::getDebugLevel()
{
  return LogSystem::Instance().getLogConfig()->getDebugLevel();
}

int LogPythonInterface::getLogInfo(LogConfig::ELogLevel level)
{
  return LogSystem::Instance().getLogConfig()->getLogInfo(level);
}

LogConfig& LogPythonInterface::getPackageLogConfig(const std::string& package)
{
  return LogSystem::Instance().getPackageLogConfig(package);
}

void LogPythonInterface::addLogFile(const std::string& filename, bool append)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionTxtFile(filename, append)));
}

void LogPythonInterface::addLogConsole(bool color)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionIOStream(std::cout, color)));
}

void LogPythonInterface::reset()
{
  LogSystem::Instance().resetLogConnections();
}

void LogPythonInterface::zeroCounters()
{
  LogSystem::Instance().resetMessageCounter();
}


/** Return dict containing message counters */
dict LogPythonInterface::getLogStatistics()
{
  dict returnDict;
  LogSystem& logSys = LogSystem::Instance();
  for (int iLevel = 0; iLevel < LogConfig::c_Default; ++iLevel) {
    LogConfig::ELogLevel logLevel = static_cast<LogConfig::ELogLevel>(iLevel);
    returnDict[logLevel] = logSys.getMessageCounter(logLevel);
  }
  return returnDict;
}

/** Create overloads since default arguments are lost in C++ */
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLogFile_overloads, addLogFile, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLogConsole_overloads, addLogConsole, 0, 1)

/** Expose python api */
void LogPythonInterface::exposePythonAPI()
{
  //Create a static instance which will serve as proxy between python and LogSystem calls
  static LogPythonInterface interface;

  scope global;

  //needed to get line numbers etc. when using B2INFO() and friends in Python
  global.attr("inspect") = import("inspect");

  //Interface LogLevel enum
  enum_<LogConfig::ELogLevel>("LogLevel")
  .value(LogConfig::logLevelToString(LogConfig::c_Debug), LogConfig::c_Debug)
  .value(LogConfig::logLevelToString(LogConfig::c_Info), LogConfig::c_Info)
  .value(LogConfig::logLevelToString(LogConfig::c_Result), LogConfig::c_Result)
  .value(LogConfig::logLevelToString(LogConfig::c_Warning), LogConfig::c_Warning)
  .value(LogConfig::logLevelToString(LogConfig::c_Error), LogConfig::c_Error)
  .value(LogConfig::logLevelToString(LogConfig::c_Fatal), LogConfig::c_Fatal)
  .value(LogConfig::logLevelToString(LogConfig::c_Default), LogConfig::c_Default)
  ;

  //Interface LogInfo enum
  enum_<LogConfig::ELogInfo>("LogInfo")
  .value("LEVEL", LogConfig::c_Level)
  .value("MESSAGE", LogConfig::c_Message)
  .value("MODULE", LogConfig::c_Module)
  .value("PACKAGE", LogConfig::c_Package)
  .value("FUNCTION", LogConfig::c_Function)
  .value("FILE", LogConfig::c_File)
  .value("LINE", LogConfig::c_Line)
  .value("TIMESTAMP", LogConfig::c_Timestamp)
  ;

  //Interface LogConfig class
  class_<LogConfig>("LogConfig")
  .def(init<optional<LogConfig::ELogLevel, int> >())
  .add_property("log_level",  &LogConfig::getLogLevel,  &LogConfig::setLogLevel)
  .add_property("debug_level", &LogConfig::getDebugLevel, &LogConfig::setDebugLevel)
  .add_property("abort_level", &LogConfig::getAbortLevel, &LogConfig::setAbortLevel)
  .def("set_log_level", &LogConfig::setLogLevel)
  .def("set_debug_level", &LogConfig::setDebugLevel)
  .def("set_abort_level", &LogConfig::setAbortLevel)
  .def("set_info", &LogConfig::setLogInfo)
  .def("get_info", &LogConfig::getLogInfo)
  ;

  //Interface the Interface class :)
  class_<LogPythonInterface, boost::noncopyable>("LogPythonInterface", no_init)
  .add_property("log_level",  &LogPythonInterface::getLogLevel,  &LogPythonInterface::setLogLevel)
  .add_property("debug_level", &LogPythonInterface::getDebugLevel, &LogPythonInterface::setDebugLevel)
  .add_property("abort_level", &LogPythonInterface::getAbortLevel, &LogPythonInterface::setAbortLevel)
  .def("set_package", &LogPythonInterface::setPackageLogConfig)
  .def("get_package", &LogPythonInterface::getPackageLogConfig, return_value_policy<reference_existing_object>())
  .def("package", &LogPythonInterface::getPackageLogConfig, return_value_policy<reference_existing_object>())
  .def("set_info", &LogPythonInterface::setLogInfo)
  .def("get_info", &LogPythonInterface::getLogInfo)
  .def("add_file", &LogPythonInterface::addLogFile, addLogFile_overloads())
  .def("add_console", &LogPythonInterface::addLogConsole, addLogConsole_overloads())
  .def("reset", &LogPythonInterface::reset)
  .def("zero_counters", &LogPythonInterface::zeroCounters)
  .def_readonly("log_stats", &LogPythonInterface::getLogStatistics)
  ;

  def("B2DEBUG", &LogPythonInterface::logDebug);
  def("B2INFO", &LogPythonInterface::logInfo);
  def("B2RESULT", &LogPythonInterface::logResult);
  def("B2WARNING", &LogPythonInterface::logWarning);
  def("B2ERROR", &LogPythonInterface::logError);
  def("B2FATAL", &LogPythonInterface::logFatal);

  //Create instance of interface class in pybasf2 module scope
  global.attr("logging") = object(ptr(&interface));
}

//
//This macro is a wrapper around the generic B2LOGMESSAGE macro to supply most
//of the arguments using information from the python interpreter. It is only
//used by the log* Messages of the LogPythonInterface to show meaningful log
//message information for messages sent from the steering file
//
#define PYTHON_LOG(loglevel, debuglevel, text) \
  B2LOGMESSAGE(loglevel, debuglevel, text, "steering", \
               extract<std::string>(eval("inspect.currentframe().f_back.f_code.co_name")), \
               extract<std::string>(eval("inspect.currentframe().f_back.f_code.co_filename")), \
               extract<int>(eval("inspect.currentframe().f_back.f_lineno")))

#define PYTHON_LOG_IFENABLED(loglevel, debuglevel, text) \
  B2LOGMESSAGE_IFENABLED(loglevel, debuglevel, text, "steering", \
                         extract<std::string>(eval("inspect.currentframe().f_back.f_code.co_name")), \
                         extract<std::string>(eval("inspect.currentframe().f_back.f_code.co_filename")), \
                         extract<int>(eval("inspect.currentframe().f_back.f_lineno")))


void LogPythonInterface::logDebug(int level, const std::string& msg)
{
#ifndef LOG_NO_B2DEBUG
  PYTHON_LOG_IFENABLED(LogConfig::c_Debug, level, msg);
#endif
}

void LogPythonInterface::logInfo(const std::string& msg)
{
#ifndef LOG_NO_B2INFO
  PYTHON_LOG_IFENABLED(LogConfig::c_Info, 0, msg);
#endif
}

void LogPythonInterface::logResult(const std::string& msg)
{
#ifndef LOG_NO_B2RESULT
  PYTHON_LOG_IFENABLED(LogConfig::c_Result, 0, msg);
#endif
}

void LogPythonInterface::logWarning(const std::string& msg)
{
#ifndef LOG_NO_B2WARNING
  PYTHON_LOG_IFENABLED(LogConfig::c_Warning, 0, msg);
#endif
}

void LogPythonInterface::logError(const std::string& msg)
{
  PYTHON_LOG(LogConfig::c_Error, 0, msg);
}

void LogPythonInterface::logFatal(const std::string& msg)
{
  PYTHON_LOG(LogConfig::c_Fatal, 0, msg);
}
