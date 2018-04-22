/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConfig.h>
#include <framework/logging/LogMessage.h>

#include <string>
#include <vector>
#include <map>


namespace Belle2 {
  class LogConnectionBase;

  /**
   * Class for logging debug, info and error messages.
   *
   * For log messages the following preprocessor macros should be used:
   *    B2DEBUG   for code development
   *    B2INFO    for informational messages, e.g. about opened files, processed events, etc.
   *    B2RESULT  for informational summary messages, e.g. about number of selected events
   *    B2WARNING for potential problems that the user should pay attention to
   *    B2ERROR   for things that went wrong and have to be fixed
   *    B2FATAL   for situations were the program execution can not be continued
   *
   *    By default the Logger sends messages to std::cout.
   *
   *    Using preprocessor macros has two advantages.
   *        - the log statements can be removed from the code completely by redefining the macros.
   *        - they allow to add information about the location (package, function, file, line).
   *
   *    This class is designed as a singleton.
   */
  class LogSystem {
  public:
    static const unsigned int c_errorSummaryMaxLines = 10000; /**< Error log will contain at most this many lines. */

    /**
     * Static method to get a reference to the LogSystem instance.
     *
     * @return A reference to an instance of this class.
     */
    static LogSystem& Instance();

    /**
     * Adds a log connection object which is used to the send the logging messages.
     *
     * \note
     * The LogSystem class takes ownership of the log connection object.
     *
     * @param logConnection Pointer to the logging connection object. The LogSystem takes ownership of this object.
     */
    void addLogConnection(LogConnectionBase* logConnection);

    /**
     * Removes all log connections.
     */
    void resetLogConnections();

    /** Reset logging system to defaults: empty all log messages and reset connections to the default */
    void resetLogging();

    /**
     * Returns global log system configuration.
     */
    LogConfig* getLogConfig() {return &m_logConfig;}

    /**
     * Add the per package log configuration.
     * Adds a new log configuration for the given package name.
     *
     * @param package The name of the package whose log configuration should be added.
     * @param logConfig The log configuration which should be assigned to the given package.
     */
    void addPackageLogConfig(const std::string& package, const LogConfig& logConfig) { m_packageLogConfigs[package] = logConfig; }

    /**
     * Get the log configuration for the package with the given name.
     * If no package specific configuration exists a new one is created.
     *
     * @param package The name of the package whose log configuration should be returned
     * @return The log configuration of the given package
     */
    LogConfig& getPackageLogConfig(const std::string& package) { return m_packageLogConfigs[package]; }

    /**
     * Returns true if the given log level is allowed by the log system (i.e. >= the system level).
     *
     * @param level The log level which should be compared with the log level of the log system
     * @param debugLevel The level for debug messages. Only used for the debug level.
     * @param package Current package (e.g. framework). NULL for package-independent config.
     */
    bool isLevelEnabled(LogConfig::ELogLevel level, int debugLevel = 0, const char* package = nullptr) const;

    /**
     * Sends a log message using the log connection object.
     * This method should _only_ be called by the logger macros.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(LogMessage message);

    /** Resets the message counter and error log by setting all message counts to 0. */
    void resetMessageCounter();

    /**
     * Returns the number of logging calls per log level.
     *
     * @param logLevel The logging level which should be returned.
     * @return The number of message calls for the given log level.
     */
    int getMessageCounter(LogConfig::ELogLevel logLevel) const;

    /**
     * Returns the current LogConfig object used by the logging system.
     *
     * Will use the configuration of the current package, if available,
     * or the current module, if available,
     * or, otherwise, the global configuration.
     *
     * The idea is that a module can call functions in a different package,
     * and the log configuration for that package will be used.
     */
    const LogConfig& getCurrentLogConfig(const char* package = nullptr) const;

    /**
     * Returns the current log level used by the logging system.
     *
     * \sa getCurrentLogConfig()
     */
    inline LogConfig::ELogLevel getCurrentLogLevel(const char* package = nullptr) const { return getCurrentLogConfig(package).getLogLevel(); }

    /**
     * Returns the current debug level used by the logging system.
     *
     * \sa getCurrentLogConfig()
     */
    inline int getCurrentDebugLevel(const char* package = nullptr) const { return getCurrentLogConfig(package).getLogLevel(); }

    /**
     * Print error/warning summary at end of execution.
     *
     * Does nothing if m_printErrorSummary is false.
     */
    void printErrorSummary();

    /** enable/disable error/warning summary after successful execution and B2FATAL.
     *
     * During normal basf2 execution, this is turned on by the Framework object.
     * For unit tests (i.e. without a Framework object), the default is used instead (off).
     */
    void enableErrorSummary(bool on) { m_printErrorSummary = on; }

    /**
     * Sets the log configuration to the given module log configuration and sets the module name
     * This method should _only_ be called by the EventProcessor.
     *
     * @param moduleLogConfig Pointer to the logging configuration object of the module.
     *                        Set to NULL to use the global log configuration.
     * @param moduleName Name of the module.
     */
    void updateModule(const LogConfig* moduleLogConfig = nullptr, const std::string& moduleName = "") { m_moduleLogConfig = moduleLogConfig; m_moduleName = moduleName; }

  private:
    std::vector<LogConnectionBase*> m_logConnections;     /**< Stores the pointers to the log connection objects. (owned by us) */
    LogConfig m_logConfig; /**< The global log system configuration. */
    const LogConfig* m_moduleLogConfig; /**< log config of current module */
    std::string m_moduleName;                             /**< The current module name. */
    std::map<std::string, LogConfig> m_packageLogConfigs; /**< Stores the log configuration objects for packages. */
    bool m_printErrorSummary;                             /**< Wether to re-print errors-warnings encountered during execution at the end. */
    std::vector<LogMessage> m_errorLog;                   /**< Save errors/warnings for summary. */

    int m_messageCounter[LogConfig::c_Default]; /**< Counts the number of messages sent per message level. */

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    LogSystem();

    /** Disable/Hide the copy constructor. */
    LogSystem(const LogSystem&) = delete;

    /** Disable/Hide the copy assignment operator. */
    LogSystem& operator=(const LogSystem&) = delete;

    /** The LogSystem destructor. */
    ~LogSystem();

    /**
     * Increases the counter for the called message level by one.
     *
     * @param logLevel The logging level which should be increased by one.
     */
    void incMessageCounter(LogConfig::ELogLevel logLevel);
  };

} //end of namespace Belle2
