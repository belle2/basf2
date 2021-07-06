/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/pybasf2/LogPythonInterface.h>

#include <framework/logging/LogConnectionFilter.h>
#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogConnectionJSON.h>
#include <framework/logging/LogConnectionUDP.h>
#include <framework/logging/LogConnectionConsole.h>
#include <framework/logging/LogVariableStream.h>
#include <framework/logging/LogSystem.h>

#include <framework/core/Environment.h>

#include <iostream>
#include <string>
#include <map>
#include <utility>

using namespace std;
using namespace Belle2;
using namespace boost::python;

void LogPythonInterface::setLogLevel(LogConfig::ELogLevel level)
{
  auto overrideLevel = (LogConfig::ELogLevel)Environment::Instance().getLogLevelOverride();
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

void LogPythonInterface::setMaxMessageRepetitions(unsigned repetitions)
{
  LogSystem::Instance().setMaxMessageRepetitions(repetitions);
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

unsigned LogPythonInterface::getMaxMessageRepetitions() const
{
  return LogSystem::Instance().getMaxMessageRepetitions();
}

void LogPythonInterface::addLogJSON(bool complete)
{
  LogSystem::Instance().addLogConnection(new LogConnectionJSON(complete));
}

void LogPythonInterface::addLogUDP(const std::string& hostname, unsigned short port)
{
  LogSystem::Instance().addLogConnection(new LogConnectionUDP(hostname, port));
}

void LogPythonInterface::addLogFile(const std::string& filename, bool append)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionTxtFile(filename, append)));
}

void LogPythonInterface::addLogConsole()
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionConsole(STDOUT_FILENO)));
}

void LogPythonInterface::addLogConsole(bool color)
{
  LogSystem::Instance().addLogConnection(new LogConnectionFilter(new LogConnectionConsole(STDOUT_FILENO, color)));
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

void LogPythonInterface::setPythonLoggingEnabled(bool enabled) const
{
  LogConnectionConsole::setPythonLoggingEnabled(enabled);
}

bool LogPythonInterface::getPythonLoggingEnabled() const
{
  return LogConnectionConsole::getPythonLoggingEnabled();
}

void LogPythonInterface::setEscapeNewlinesEnabled(bool enabled) const
{
  LogConnectionConsole::setEscapeNewlinesEnabled(enabled);
}

bool LogPythonInterface::getEscapeNewlinesEnabled() const
{
  return LogConnectionConsole::getEscapeNewlinesEnabled();
}

/** Return dict containing message counters */
dict LogPythonInterface::getLogStatistics()
{
  dict returnDict;
  const LogSystem& logSys = LogSystem::Instance();
  for (int iLevel = 0; iLevel < LogConfig::c_Default; ++iLevel) {
    auto logLevel = static_cast<LogConfig::ELogLevel>(iLevel);
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
    return LogConnectionConsole::terminalSupportsColors(STDOUT_FILENO);
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

  The agreed values for ``debug_level`` are

  * **0-9** for user code. These numbers are reserved for user analysis code and
    may not be used by any part of basf2.
  * **10-19** for analysis package code. The use case is that a user wants to debug
    problems in analysis jobs with the help of experts.

  * **20-29** for simulation/reconstruction code.
  * **30-39** for core framework code.

  .. note:: The default maximum debug level which will be shown when
            running ``basf2 --debug`` without any argument for ``--debug`` is **10**


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
  .def(init<bp::optional<LogConfig::ELogLevel, int> >())
  .add_property("log_level",  &LogConfig::getLogLevel,  &LogConfig::setLogLevel, "set or get the current log level")
  .add_property("debug_level", &LogConfig::getDebugLevel, &LogConfig::setDebugLevel, "set or get the current debug level")
  .add_property("abort_level", &LogConfig::getAbortLevel, &LogConfig::setAbortLevel,
                "set or get the severity which causes program abort")
  .def("set_log_level", &LogConfig::setLogLevel, args("log_level"), R"DOC(
Set the minimum log level to be shown. Messages with a log level below this value will not be shown at all.

.. warning: Message with a level of `ERROR <LogLevel.ERROR>` or higher will always be shown and cannot be silenced.
)DOC")
  .def("set_debug_level", &LogConfig::setDebugLevel, args("debug_level"), R"DOC(
Set the maximum debug level to be shown. Any messages with log level `DEBUG <LogLevel.DEBUG>` and a larger debug level will not be shown.

.. seealso: the documentation of `DEBUG <LogLevel.DEBUG>` for suitable values
)DOC")
  .def("set_abort_level", &LogConfig::setAbortLevel, args("abort_level"), R"DOC(
Set the severity which causes program abort.

This can be set to a `LogLevel` which will cause the processing to be aborted if
a message with the given level or higher is encountered. The default is
`FATAL <LogLevel.FATAL>`. It cannot be set any higher but can be lowered.
)DOC")
  .def("set_info", &LogConfig::setLogInfo, args("log_level", "log_info"),
       "set the bitmask of LogInfo members to show when printing messages for a given log level")
  .def("get_info", &LogConfig::getLogInfo, args("log_level"),
       "get the current bitmask of which parts of the log message will be printed for a given log level")
  ;

  void (LogPythonInterface::*addLogConsole)(bool) = &LogPythonInterface::addLogConsole;

  //Interface the Interface class :)
  class_<LogPythonInterface, std::shared_ptr<LogPythonInterface>, boost::noncopyable>("LogPythonInterface", R"(
Logging configuration (for messages generated from C++ or Python), available as a global `basf2.logging` object in Python. See also `basf2.set_log_level()` and `basf2.set_debug_level()`.

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
  .add_property("log_level",  &LogPythonInterface::getLogLevel,  &LogPythonInterface::setLogLevel, R"DOC(
Attribute for setting/getting the current `log level <basf2.LogLevel>`.
Messages with a lower level are ignored.

.. warning: Message with a level of `ERROR <LogLevel.ERROR>` or higher will always be shown and cannot be silenced.
)DOC")
  .add_property("debug_level", &LogPythonInterface::getDebugLevel, &LogPythonInterface::setDebugLevel,
                "Attribute for getting/setting the debug level. If debug messages are enabled, their level needs to be at least this high to be printed. Defaults to 100.")
  .add_property("abort_level", &LogPythonInterface::getAbortLevel, &LogPythonInterface::setAbortLevel,
                "Attribute for setting/getting the `log level <basf2.LogLevel>` at which to abort processing. Defaults to `FATAL <LogLevel.FATAL>` but can be set to a lower level in rare cases.")
  .add_property("max_repetitions", &LogPythonInterface::getMaxMessageRepetitions, &LogPythonInterface::setMaxMessageRepetitions, R"DOC(
Set the maximum amount of times log messages with the same level and message text
(excluding variables) will be repeated before it is suppressed. Suppressed messages
will still be counted but not shown for the remainder of the processing.

This affects messages with the same text but different ref:`logging_logvariables`.
If the same log message is repeated frequently with different variables all of
these will be suppressed after the given amount of repetitions.

.. versionadded:: release-05-00-00
)DOC")

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

Parameters:
  log_level (LogLevel): log level for which to set the display info
  log_info (int): Bitmask of `basf2.LogInfo` constants.)DOCSTRING")
  .def("get_info", &LogPythonInterface::getLogInfo, args("log_level"), "Get info to print for given log level.\n\n"
       "Parameters:\n  log_level (basf2.LogLevel): Log level for which to get the display info")
  .def("add_file", &LogPythonInterface::addLogFile, (bp::arg("filename"), bp::arg("append") = false),
       R"DOCSTRING(Write log output to given file. (In addition to existing outputs)\n\n"

Parameters:
  filename (str): Filename to to write log messages into
  append (bool): If set to True the file will be truncated before writing new messages.)DOCSTRING")
  .def("add_console", addLogConsole,
       addLogConsole_overloads(args("enable_color"), "Write log output to console. (In addition to existing outputs). "
                               "If ``enable_color`` is not specified color will be enabled if supported"))
  .def("add_json", &LogPythonInterface::addLogJSON, (bp::arg("complete_info") = false), R"DOCSTRING(
Write log output to console, but format log messages as json objects for
simplified parsing by other tools.  Each log message will be printed as a one
line JSON object.

.. versionadded:: release-03-00-00

Parameters:
   complete_info (bool): If this is set to True the complete log information is printed regardless of the `LogInfo` setting.

See Also:
   `add_console()`, `set_info()`
)DOCSTRING")
  .def("add_udp", &LogPythonInterface::addLogUDP, (bp::arg("hostname"), bp::arg("port")), R"DOCSTRING(
    Send the log output as a JSON object to the given hostname and port via UDP.

.. versionadded:: release-04-00-00

Parameters:
   hostname (str): The hostname to send the message to. If it can not be resolved, an exception will be thrown.
   port (int): The port on the host to send the message via UDP.

See Also:
   `add_json()`
)DOCSTRING")
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
  .add_property("enable_python_logging",  &LogPythonInterface::getPythonLoggingEnabled,
                &LogPythonInterface::setPythonLoggingEnabled, R"DOCSTRING(
Enable or disable logging via python. If this is set to true than log messages
will be sent via `sys.stdout`. This is probably slightly slower but is useful
when running in jupyter notebooks or when trying to redirect stdout in python
to a buffer. This setting affects all log connections to the
console.

.. versionadded:: release-03-00-00)DOCSTRING")
  .add_property("enable_escape_newlines", &LogPythonInterface::getEscapeNewlinesEnabled,
                &LogPythonInterface::setEscapeNewlinesEnabled, R"DOCSTRING(
Enable or disable escaping of newlines in log messages to the console. If this
is set to true than any newline character in log messages printed to the console
will be replaced by a "\n" to ensure that every log messages fits exactly on one line.

.. versionadded:: release-04-02-00)DOCSTRING")
  ;

  //Expose Logging object
  std::shared_ptr<LogPythonInterface> initguard{new LogPythonInterface()};
  scope().attr("logging") = initguard;

  //Add all the logging functions. To handle arbitrary keyword arguments we add
  //them as raw functions. However it seems setting the docstring needs to be
  //done manually in this case. So create function objects, add to namespace,
  //set docstring ...

  const std::string common_doc = R"DOCSTRING(
All additional positional arguments are converted to strings and concatenated
to the log message. All keyword arguments are added to the function as
:ref:`logging_logvariables`.)DOCSTRING";

  auto logDebug = raw_function(&LogPythonInterface::logDebug);
  def("B2DEBUG", logDebug);
  setattr(logDebug, "__doc__", "B2DEBUG(debugLevel, message, *args, **kwargs)\n\n"
          "Print a `DEBUG <basf2.LogLevel.DEBUG>` message. "
          "The first argument is the `debug_level <basf2.LogLevel.DEBUG>`. " +
          common_doc);

  auto logInfo = raw_function(&LogPythonInterface::logInfo);
  def("B2INFO", logInfo);
  setattr(logInfo, "__doc__", "B2INFO(message, *args, **kwargs)\n\n"
          "Print a `INFO <basf2.LogLevel.INFO>` message. " + common_doc);

  auto logResult = raw_function(&LogPythonInterface::logResult);
  def("B2RESULT", logResult);
  setattr(logResult, "__doc__", "B2RESULT(message, *args, **kwargs)\n\n"
          "Print a `RESULT <basf2.LogLevel.RESULT>` message. " + common_doc
          + "\n\n.. deprecated:: release-01-00-00\n    use `B2INFO()` instead");

  auto logWarning = raw_function(&LogPythonInterface::logWarning);
  def("B2WARNING", logWarning);
  setattr(logWarning, "__doc__", "B2WARNING(message, *args, **kwargs)\n\n"
          "Print a `WARNING <basf2.LogLevel.WARNING>` message. " + common_doc);

  auto logError = raw_function(&LogPythonInterface::logError);
  def("B2ERROR", logError);
  setattr(logError, "__doc__", "B2ERROR(message, *args, **kwargs)\n\n"
          "Print a `ERROR <basf2.LogLevel.ERROR>` message. " + common_doc);

  auto logFatal = raw_function(&LogPythonInterface::logFatal);
  def("B2FATAL", logFatal);
  setattr(logFatal, "__doc__", "B2FATAL(message, *args, **kwargs)\n\n"
          "Print a `FATAL <basf2.LogLevel.FATAL>` message. " + common_doc +
          "\n\n.. note:: This also exits the programm with an error and is "
          "guaranteed to not return.");
}

namespace {
  /** small helper function to convert any python object to a string representation */
  std::string pythonObjectToString(const boost::python::object& obj)
  {
    return boost::python::extract<std::string>(obj.attr("__str__")());
  }

  /** small helper function to convert a python dict containing arbitrary
   * objects to a std::map<string,string> by using the `str()` operator in
   * python.
   */
  auto pythonDictToMap(const dict& d)
  {
    std::map<std::string, std::string> result;
    if (d.is_none()) return result;
    const auto items = d.items();
    const int size = len(d);
    for (int i = 0; i < size; ++i) {
      const auto key = pythonObjectToString(items[i][0]);
      const auto val = pythonObjectToString(items[i][1]);
      result.emplace(std::make_pair(key, val));
    }
    return result;
  }

  /** Dispatch a log message from a generic raw python function call:
   * Concatenate all the positional arguments and add the keyword arguments as
   * log stream variables. In case of debug messages the first argument is
   * treated as the debug level.
   */
  void dispatchMessage(LogConfig::ELogLevel logLevel, boost::python::tuple args, const boost::python::dict& kwargs)
  {
    int debugLevel = 0;
    const int firstArg = logLevel == LogConfig::c_Debug ? 1 : 0;
    const int argSize = len(args);
    if (argSize - firstArg <= 0) {
      PyErr_SetString(PyExc_TypeError, ("At least " + std::to_string(firstArg + 1) + " positional arguments required").c_str());
      boost::python::throw_error_already_set();
    }
    if (logLevel == LogConfig::c_Debug) {
      boost::python::extract<int> proxy(args[0]);
      if (!proxy.check()) {
        PyErr_SetString(PyExc_TypeError, "First argument `debugLevel` must be an integer");
        boost::python::throw_error_already_set();
      }
      debugLevel = proxy;
    }
    if (logLevel >= LogConfig::c_Error || Belle2::LogSystem::Instance().isLevelEnabled(logLevel, debugLevel, "steering")) {
      //Finally we know we actually will send the message: concatenate all
      //positional arguments and convert the keyword arguments to a python dict
      stringstream message;
      int size = len(args);
      for (int i = firstArg; i < size; ++i) {
        message << pythonObjectToString(args[i]);
      }
      const auto cppKwArgs = pythonDictToMap(kwargs);
      LogVariableStream lvs(message.str(), cppKwArgs);

      // Now we also need to find out where the message came from: use the
      // inspect module to get the filename/linenumbers
      object inspect = import("inspect");
      auto frame = inspect.attr("currentframe")();
      const std::string function = extract<std::string>(frame.attr("f_code").attr("co_name"));
      const std::string file = extract<std::string>(frame.attr("f_code").attr("co_filename"));
      int line = extract<int>(frame.attr("f_lineno"));

      // Everything done, send it away
      Belle2::LogSystem::Instance().sendMessage(Belle2::LogMessage(logLevel, std::move(lvs), "steering",
                                                function, file, line, debugLevel));
    }
  }
}

boost::python::object LogPythonInterface::logDebug(boost::python::tuple args, const boost::python::dict& kwargs)
{
#ifndef LOG_NO_B2DEBUG
  dispatchMessage(LogConfig::c_Debug, std::move(args), kwargs);
#endif
  return boost::python::object();
}

boost::python::object LogPythonInterface::logInfo(boost::python::tuple args, const boost::python::dict& kwargs)
{
#ifndef LOG_NO_B2INFO
  dispatchMessage(LogConfig::c_Info, std::move(args), kwargs);
#endif
  return boost::python::object();
}

boost::python::object LogPythonInterface::logResult(boost::python::tuple args, const boost::python::dict& kwargs)
{
#ifndef LOG_NO_B2RESULT
  dispatchMessage(LogConfig::c_Result, std::move(args), kwargs);
#endif
  return boost::python::object();
}

boost::python::object LogPythonInterface::logWarning(boost::python::tuple args, const boost::python::dict& kwargs)
{
#ifndef LOG_NO_B2WARNING
  dispatchMessage(LogConfig::c_Warning, std::move(args), kwargs);
#endif
  return boost::python::object();
}

boost::python::object LogPythonInterface::logError(boost::python::tuple args, const boost::python::dict& kwargs)
{
  dispatchMessage(LogConfig::c_Error, std::move(args), kwargs);
  return boost::python::object();
}

boost::python::object LogPythonInterface::logFatal(boost::python::tuple args, const boost::python::dict& kwargs)
{
  dispatchMessage(LogConfig::c_Fatal, std::move(args), kwargs);
  std::exit(1);
  return boost::python::object();
}
