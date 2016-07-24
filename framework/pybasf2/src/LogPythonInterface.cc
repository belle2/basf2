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

void LogPythonInterface::enableErrorSummary(bool on)
{
  LogSystem::Instance().enableErrorSummary(on);
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

#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
/** Create overloads since default arguments are lost in C++ */
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLogFile_overloads, addLogFile, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLogConsole_overloads, addLogConsole, 0, 1)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

/** Expose python api */
void LogPythonInterface::exposePythonAPI()
{
  scope global;

  //Interface LogLevel enum
  enum_<LogConfig::ELogLevel>("LogLevel", "enum encapsulating all the possible log levels")
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
  class_<LogConfig>("LogConfig",
                    R"(Defines logging settings (log levels and items included in each message) for a certain context, e.g. a module or package.

.. seealso:: :func:`logging.package(str) <basf2.LogPythonInterface.package>`)")
  .def(init<optional<LogConfig::ELogLevel, int> >())
  .add_property("log_level",  &LogConfig::getLogLevel,  &LogConfig::setLogLevel, "set or get the current log level")
  .add_property("debug_level", &LogConfig::getDebugLevel, &LogConfig::setDebugLevel, "set or get the current debug level")
  .add_property("abort_level", &LogConfig::getAbortLevel, &LogConfig::setAbortLevel,
                "set or get the severity which causes program abort")
  .def("set_log_level", &LogConfig::setLogLevel, "set the log level")
  .def("set_debug_level", &LogConfig::setDebugLevel, "set the debug level")
  .def("set_abort_level", &LogConfig::setAbortLevel, "set the severity which causes program abort")
  .def("set_info", &LogConfig::setLogInfo, "set the bitmask of LogInfo members to show when printing messages")
  .def("get_info", &LogConfig::getLogInfo, "get the current bitmask of which parts of the log message will be printed")
  ;

  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  //Interface the Interface class :)
  class_<LogPythonInterface, boost::noncopyable>("LogPythonInterface",
                                                 R"(Logging configuration (for messages generated from C++ or Python), available as a global `logging` object in Python. See also :func:`basf2.set_log_level()` and :func:`basf2.set_debug_level()`.

This class exposes a object called ``logging`` to the python interface. With
this object it is possible to set all properties of the logging system
directly in the steering file in a consistent manner This class also
exposes the :class:`basf2.LogConfig` class as well as the :class:`basf2.LogLevel`
and :class:`basf2.LogInfo` enums to make setting of properties more transparent
by using the names and not just the values.  To set or get the log level,
one can simply do:

>>> logging.log_level = LogLevel.FATAL
>>> print("Logging level set to", logging.log_level)
FATAL

This module also allows to send log messages directly from python to ease
consistent error reporting througout the framework

>>> B2WARNING("This is a warning message")

For all features, see b2logging.py in the framework/examples folder)")
  .add_property("log_level",  &LogPythonInterface::getLogLevel,  &LogPythonInterface::setLogLevel,
                "Attribute for setting/getting the current log level (:class:`basf2.LogLevel`). Messages with a lower level are ignored.")
  .add_property("debug_level", &LogPythonInterface::getDebugLevel, &LogPythonInterface::setDebugLevel,
                "Attribute for getting/setting the debug log level. If debug messages are enabled, their level needs to be at least this high to be printed. Defaults to 100.")
  .add_property("abort_level", &LogPythonInterface::getAbortLevel, &LogPythonInterface::setAbortLevel,
                "Attribute for setting/getting the log level (:class:`basf2.LogLevel`) at which to abort processing. Defaults to FATAL.")
  .def("set_package", &LogPythonInterface::setPackageLogConfig,
       "Set :class:`basf2.LogConfig` for given package, see also :func:`package() <basf2.LogPythonInterface.package>`.")
  .def("package", &LogPythonInterface::getPackageLogConfig, return_value_policy<reference_existing_object>(),
       R"(Get :class:`basf2.LogConfig` for given package.

  >>> logging.package('svd').debug_level = 10
  >>> logging.package('svd').set_info(LogLevel.INFO, LogInfo.LEVEL | LogInfo.MESSAGE | LogInfo.FILE)
      )")
  .def("set_info", &LogPythonInterface::setLogInfo,
       "Set info to print for given log level. Should be ORed combination of :class:`basf2.LogInfo`.")
  .def("get_info", &LogPythonInterface::getLogInfo, "Get info to print for given log level.")
  .def("add_file", &LogPythonInterface::addLogFile,
       addLogFile_overloads("Write log output to given file. (In addition to existing outputs)"))
  .def("add_console", &LogPythonInterface::addLogConsole,
       addLogConsole_overloads("Write log output to console. (In addition to existing outputs)"))
  .def("terminal_supports_colors", &LogConnectionIOStream::terminalSupportsColors)
  .staticmethod("terminal_supports_colors")
  .def("reset", &LogPythonInterface::reset,
       "Remove all configured logging outputs. You can then configure your own via :func:`add_file() <basf2.LogPythonInterface.add_file>` or :func:`add_console() <basf2.LogPythonInterface.add_console>`")
  .def("zero_counters", &LogPythonInterface::zeroCounters, "Reset the per-level message counters.")
  .def_readonly("log_stats", &LogPythonInterface::getLogStatistics, "Returns dictionary with message counters.")
  .def("enable_summary", &LogPythonInterface::enableErrorSummary, args("on"),
       "Enable or disable the error summary printed at the end of processing. "
       "Expects one argument wether or not the summary should be shown")
  ;

  def("B2DEBUG", &LogPythonInterface::logDebug, args("debuglevel", "message"), "create a debug message with the given debug level");
  def("B2INFO", &LogPythonInterface::logInfo, args("message"), "create an info message");
  def("B2RESULT", &LogPythonInterface::logResult, args("message"), "create an info message");
  def("B2WARNING", &LogPythonInterface::logWarning, args("message"), "create a warning message");
  def("B2ERROR", &LogPythonInterface::logError, args("message"), "create an error message");
  def("B2FATAL", &LogPythonInterface::logFatal, args("message"), "create a fatal error message");
}

//
//This macro is a wrapper around the generic _B2LOGMESSAGE macro to supply most
//of the arguments using information from the python interpreter. It is only
//used by the log* Messages of the LogPythonInterface to show meaningful log
//message information for messages sent from the steering file
//
//inspect is needed to get line numbers etc. when using B2INFO() and friends in Python
#define PYTHON_LOG(loglevel, debuglevel, text) \
  object inspectDict = import("inspect").attr("__dict__"); \
  _B2LOGMESSAGE(loglevel, debuglevel, text, "steering", \
                extract<std::string>(eval("currentframe().f_back.f_code.co_name", inspectDict)), \
                extract<std::string>(eval("currentframe().f_back.f_code.co_filename", inspectDict)), \
                extract<int>(eval("currentframe().f_back.f_lineno", inspectDict)))

#define PYTHON_LOG_IFENABLED(loglevel, debuglevel, text) \
  object inspectDict = import("inspect").attr("__dict__"); \
  _B2LOGMESSAGE_IFENABLED(loglevel, debuglevel, text, "steering", \
                          extract<std::string>(eval("currentframe().f_back.f_code.co_name", inspectDict)), \
                          extract<std::string>(eval("currentframe().f_back.f_code.co_filename", inspectDict)), \
                          extract<int>(eval("currentframe().f_back.f_lineno", inspectDict)))


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
