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
#include <framework/logging/LogConnectionFileDescriptor.h>

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

void LogPythonInterface::addLogConsole()
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionFileDescriptor(STDOUT_FILENO)));
}

void LogPythonInterface::addLogConsole(bool color)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionFileDescriptor(STDOUT_FILENO, color)));
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

namespace {
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
  /** Create overloads since default arguments are lost in C++ */
  BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(addLogConsole_overloads, addLogConsole, 0, 1)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

  bool terminalSupportsColors()
  {
    return LogConnectionFileDescriptor::terminalSupportsColors(STDOUT_FILENO);
  }
}

/** Expose python api */
void LogPythonInterface::exposePythonAPI()
{
  // to avoid confusion between std::arg and boost::python::arg we want a shorthand namespace as well
  namespace bp = boost::python;
  scope global;
  docstring_options options(true, true, false); //userdef, py sigs, c++ sigs

  //Interface LogLevel enum
  enum_<LogConfig::ELogLevel>("LogLevel", R"DOCSTRING(Class for all possible log levels

.. attribute:: DEBUG

  The lowest possible severity meant for expert only information and disabled
  by default. In contrast to all other log levels DEBUG messages have an
  additional numeric indication of their priority called the ``debug_level`` to
  allow for different levels of verbosity.

.. attribute:: INFO

  Used for informational messages which are of use for the average user but not
  very important. Should be used very sparsely, everything which is of no
  interest to the average user should be a debug message.

.. attribute:: RESULT

  Informational message which don't indicate an error condition but are more
  important than a mere information. For example the calculated cross section
  or the output file name.

  .. deprecated:: release-01-00-00
     use `INFO <basf2.LogLevel.INFO>` messages instead

.. attribute:: WARNING

  For messages which indicate something which is not correct but not fatal to
  the processing. This should **not** be used to make informational messages
  more prominent and they should not be ignored by the user but they are not
  critical.

.. attribute:: ERROR

  For messages which indicate a clear error condition which needs to be
  recovered. If error messages are produced before event processing is started
  the processing will be aborted. During processing errors don't lead to a stop
  of the processing but still indicate a problem.

.. attribute:: FATAL

  For errors so severe that no recovery is possible. Emitting a fatal error
  will always stop the processing and the `B2FATAL` function is guaranteed to
  not return.
)DOCSTRING")
  .value(LogConfig::logLevelToString(LogConfig::c_Debug), LogConfig::c_Debug)
  .value(LogConfig::logLevelToString(LogConfig::c_Info), LogConfig::c_Info)
  .value(LogConfig::logLevelToString(LogConfig::c_Result), LogConfig::c_Result)
  .value(LogConfig::logLevelToString(LogConfig::c_Warning), LogConfig::c_Warning)
  .value(LogConfig::logLevelToString(LogConfig::c_Error), LogConfig::c_Error)
  .value(LogConfig::logLevelToString(LogConfig::c_Fatal), LogConfig::c_Fatal)
  .value(LogConfig::logLevelToString(LogConfig::c_Default), LogConfig::c_Default)
  ;

  //Interface LogInfo enum
  enum_<LogConfig::ELogInfo>("LogInfo", R"DOCSTRING(The different fields of a log message.

These fields can be used as a bitmask to configure the appearance of log messages.

.. attribute:: LEVEL

  The severity of the log message, one of `basf2.LogLevel`

.. attribute:: MESSAGE

  The actual log message

.. attribute:: MODULE

  The name of the module active when the message was emitted. Can be empty if
  no module was active (before/after processing or outside of the normal event
  loop)

.. attribute:: PACKAGE

  The package the code that emitted the message belongs to. This is empty for
  messages emitted by python scripts

.. attribute:: FUNCTION

  The function name that emitted the message

.. attribute:: FILE

  The filename containing the code emitting the message

.. attribute:: LINE

  The line number in the file emitting the message
)DOCSTRING")
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

.. seealso:: `logging.package(str) <basf2.LogPythonInterface.package>`)")
  .def(init<optional<LogConfig::ELogLevel, int> >())
  .add_property("log_level",  &LogConfig::getLogLevel,  &LogConfig::setLogLevel, "set or get the current log level")
  .add_property("debug_level", &LogConfig::getDebugLevel, &LogConfig::setDebugLevel, "set or get the current debug level")
  .add_property("abort_level", &LogConfig::getAbortLevel, &LogConfig::setAbortLevel,
                "set or get the severity which causes program abort")
  .def("set_log_level", &LogConfig::setLogLevel, args("log_level"), "set the log level")
  .def("set_debug_level", &LogConfig::setDebugLevel, args("debug_level"), "set the debug level")
  .def("set_abort_level", &LogConfig::setAbortLevel, args("abort_level"), "set the severity which causes program abort")
  .def("set_info", &LogConfig::setLogInfo, args("log_level", "log_info"),
       "set the bitmask of LogInfo members to show when printing messages for a given log level")
  .def("get_info", &LogConfig::getLogInfo, args("log_level"),
       "get the current bitmask of which parts of the log message will be printed for a given log level")
  ;

  void (LogPythonInterface::*addLogConsole)(bool) = &LogPythonInterface::addLogConsole;

  //Interface the Interface class :)
  class_<LogPythonInterface, boost::noncopyable>("LogPythonInterface",
                                                 R"(Logging configuration (for messages generated from C++ or Python), available as a global `basf2.logging` object in Python. See also `basf2.set_log_level()` and `basf2.set_debug_level()`.

This class exposes a object called `logging` to the python interface. With
this object it is possible to set all properties of the logging system
directly in the steering file in a consistent manner This class also
exposes the `LogConfig` class as well as the `LogLevel`
and `LogInfo` enums to make setting of properties more transparent
by using the names and not just the values.  To set or get the log level,
one can simply do:

>>> logging.log_level = LogLevel.FATAL
>>> print("Logging level set to", logging.log_level)
FATAL

This module also allows to send log messages directly from python to ease
consistent error reporting throughout the framework

>>> B2WARNING("This is a warning message")

.. seealso::

   For all features, see :download:`b2logging.py </framework/examples/b2logging.py>`)")
  .add_property("log_level",  &LogPythonInterface::getLogLevel,  &LogPythonInterface::setLogLevel,
                "Attribute for setting/getting the current `log level <basf2.LogLevel>`. Messages with a lower level are ignored.")
  .add_property("debug_level", &LogPythonInterface::getDebugLevel, &LogPythonInterface::setDebugLevel,
                "Attribute for getting/setting the debug level. If debug messages are enabled, their level needs to be at least this high to be printed. Defaults to 100.")
  .add_property("abort_level", &LogPythonInterface::getAbortLevel, &LogPythonInterface::setAbortLevel,
                "Attribute for setting/getting the `log level <basf2.LogLevel>` at which to abort processing. Defaults to `FATAL <LogLevel.FATAL>` but can be set to a lower level in rare cases.")
  .def("set_package", &LogPythonInterface::setPackageLogConfig, args("package", "config"),
       "Set `basf2.LogConfig` for given package, see also `package() <basf2.LogPythonInterface.package>`.")
  .def("package", &LogPythonInterface::getPackageLogConfig, return_value_policy<reference_existing_object>(), args("package"),
       R"(Get the `LogConfig` for given package to set detailed logging pararameters for this package.

  >>> logging.package('svd').debug_level = 10
  >>> logging.package('svd').set_info(LogLevel.INFO, LogInfo.LEVEL | LogInfo.MESSAGE | LogInfo.FILE)
      )")
  .def("set_info", &LogPythonInterface::setLogInfo, args("log_level", "log_info"),
       R"DOCSTRING(Set info to print for given log level. Should be an OR combination of `basf2.LogInfo` constants.
As an example, to show only the level and text for all debug messages one could use

>>> basf2.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)

:param LogLevel log_level: log level for which to set the display info
:param int log_info: Bitmask of `basf2.LogInfo` constants.)DOCSTRING")
  .def("get_info", &LogPythonInterface::getLogInfo, args("log_level"), "Get info to print for given log level.\n\n"
       ":param basf2.LogLevel log_level: Log level for which to get the display info")
  .def("add_file", &LogPythonInterface::addLogFile, (bp::arg("filename"), bp::arg("append") = false),
       "Write log output to given file. (In addition to existing outputs)\n\n"
       ":param str filename: Filename to to write log messages into\n"
       ":param bool append: If set to True the file will be truncated before writing new messages.")
  .def("add_console", addLogConsole,
       addLogConsole_overloads(args("enable_color"), "Write log output to console. (In addition to existing outputs). "
                               "If ``enable_color`` is not specified color will be enabled if supported"))
  .def("terminal_supports_colors", &terminalSupportsColors, "Returns true if the terminal supports colored output")
  .staticmethod("terminal_supports_colors")
  .def("reset", &LogPythonInterface::reset, "Remove all configured logging outputs. "
       "You can then configure your own via `add_file() <basf2.LogPythonInterface.add_file>` "
       "or `add_console() <basf2.LogPythonInterface.add_console>`")
  .def("zero_counters", &LogPythonInterface::zeroCounters, "Reset the per-level message counters.")
  .def_readonly("log_stats", &LogPythonInterface::getLogStatistics, "Returns dictionary with message counters.")
  .def("enable_summary", &LogPythonInterface::enableErrorSummary, args("on"),
       "Enable or disable the error summary printed at the end of processing. "
       "Expects one argument whether or not the summary should be shown")
  ;

  def("B2DEBUG", &LogPythonInterface::logDebug, args("debuglevel", "message"),
      "create a `DEBUG <basf2.LogLevel.DEBUG>` message with the given debug level");
  def("B2INFO", &LogPythonInterface::logInfo, args("message"),
      "create an `INFO <basf2.LogLevel.INFO>` message");
  def("B2RESULT", &LogPythonInterface::logResult, args("message"),
      "create an `RESULT <basf2.LogLevel.RESULT>` message");
  def("B2WARNING", &LogPythonInterface::logWarning, args("message"),
      "create a `WARNING <basf2.LogLevel.WARNING>` message");
  def("B2ERROR", &LogPythonInterface::logError, args("message"),
      "create an `ERROR <basf2.LogLevel.ERROR>` message");
  def("B2FATAL", &LogPythonInterface::logFatal, args("message"),
      "create a `FATAL <basf2.LogLevel.FATAL>` error message and abort processing");
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
