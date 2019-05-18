/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Thomas Kuhr, Thomas Hauth                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConfig.h>

#include <boost/python/dict.hpp>

#include <string>

namespace Belle2 {

  /**
   * Thin wrapper to expose a usable interface to the logging framework in python
   *
   * This class exposes a object called "logging" to the python interface. With
   * this object it is possible to set all properties of the logging system
   * directly in the steering file in a consistent manner This class also
   * exposes the LogConfig class as well as the LogConfig::ELogLevel and
   * LogConfig::ELogInfo enums to make setting of properties more transparent
   * by using the names and not just the values.  To set or get the log level,
   * one can simply do:
   *
   * >>> logging.log_level = LogLevel.FATAL
   * >>> print "Logging level set to", logging.log_level
   *
   * This module also allows to send log messages directly from python to ease
   * consistent error reporting througout the framework
   *
   * >>> B2WARNING("This is a warning message");
   *
   * For all features, see b2logging.py in the framework/examples folder
   */
  class LogPythonInterface {
  public:
    /** ctor*/
    LogPythonInterface() {};

    /**
     * Set the log level.
     *
     * @param level The log level
     */
    void setLogLevel(LogConfig::ELogLevel level);

    /**
     * Set the abort log level.
     *
     * @param level The abort log level
     */
    void setAbortLevel(LogConfig::ELogLevel level);

    /**
     * Set the debug messaging level.
     *
     * @param level The debug level
     */
    void setDebugLevel(int level);

    /**
     * Set the printed log information for the given level.
     *
     * @param level The log level
     * @param info The log information that should be printed
     */
    void setLogInfo(LogConfig::ELogLevel level, int info);

    /**
     * Set LogConfig for a given package
     *
     * @param package The package name
     * @param config The log configuration object
     */
    void setPackageLogConfig(const std::string& package, const LogConfig& config);

    /**
     * Get the log level
     *
     * @return The log level
     */
    LogConfig::ELogLevel getLogLevel();

    /**
     * Get the abort level
     *
     * @return The abort level
     */
    LogConfig::ELogLevel getAbortLevel();

    /**
     * Get the debug level
     *
     * @return The debug level
     */
    int getDebugLevel();

    /**
     * Get the printed log information for the given level.
     *
     * @param level The log level
     * @return The log information that should be printed (see LogConfig::ELogInfo)
     */
    int getLogInfo(LogConfig::ELogLevel level);

    /**
     * Get the LogConfig for the given package
     *
     * @param package The package name
     * @return The log configuration object
     */
    LogConfig& getPackageLogConfig(const std::string& package);


    /**
     * Add a file as output connection
     *
     * @param filename The text file name
     * @param append Flag whether output should be appended to an existing file
     */
    void addLogFile(const std::string& filename, bool append = false);

    /**
     * Add the console as output connection. Color will be used automatically if supported
     */
    void addLogConsole();

    /**
     * Add the console as output connection but print the log messages as json
     * objects so that they can be parsed easily
     * @param complete if true ignore the log info configuration and print all
     *                 parts of the log message.
     */
    void addLogJSON(bool complete);

    /**
     * Add the console as output connection
     *
     * @param color Flag whether color should be used
     */
    void addLogConsole(bool color);

    /**
     * Reset logging connections
     */
    void reset();

    /**
     * Reset logging counters
     */
    void zeroCounters();

    /** Enable/Disable error summary */
    void enableErrorSummary(bool on);

    /**
     * Set flag if logging should be done via python `sys.stdout`
     */
    void setPythonLoggingEnabled(bool enabled) const;

    /**
     * Get flag if logging should be done via python `sys.stdout`
     */
    bool getPythonLoggingEnabled() const;

    /**
     * Produce debug message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logDebug(boost::python::tuple args, const boost::python::dict& kwargs);
    /**
     * Produce info message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logInfo(boost::python::tuple args, const boost::python::dict& kwargs);

    /**
     * Produce result message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logResult(boost::python::tuple args, const boost::python::dict& kwargs);
    /**
     * Produce warning message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logWarning(boost::python::tuple args, const boost::python::dict& kwargs);
    /**
     * Produce error message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logError(boost::python::tuple args, const boost::python::dict& kwargs);

    /**
     * Produce fatal message
     *
     * @param args positional arguments, concatenated to form message
     * @param kwargs keyword arguments to be converted to log variables
     */
    static boost::python::object logFatal(boost::python::tuple args, const boost::python::dict& kwargs);

    /** return dict with log statistics */
    boost::python::dict getLogStatistics();
    /** expose python API */
    static void exposePythonAPI();
  protected:
  };

} // end namespace Belle2
