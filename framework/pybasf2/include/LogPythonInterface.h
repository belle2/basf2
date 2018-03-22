/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Thomas Kuhr                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/python/dict.hpp>

#include <framework/logging/LogConfig.h>

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
     * Produce debug message
     *
     * @param level The debug level
     * @param msg The debug message text
     */
    static void logDebug(int level, const std::string& msg);

    /**
     * Produce info message
     *
     * @param msg The info message text
     */
    static void logInfo(const std::string& msg);

    /**
     * Produce result message
     *
     * @param msg The result message text
     */
    static void logResult(const std::string& msg);

    /**
     * Produce warning message
     *
     * @param msg The warning message
     */
    static void logWarning(const std::string& msg);

    /**
     * Produce error message
     *
     * @param msg The error message text
     */
    static void logError(const std::string& msg);

    /**
     * Produce fatal message
     *
     * @param msg The fatal message text
     */
    static void logFatal(const std::string& msg);


    /** return dict with log statistics */
    boost::python::dict getLogStatistics();
    /** expose python API */
    static void exposePythonAPI();
  protected:
  };

} // end namespace Belle2
