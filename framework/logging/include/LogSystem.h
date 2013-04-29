/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGSYSTEM_H_
#define LOGSYSTEM_H_

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

    /**
     * Returns the log system configuration.
     *
     * @return The log system configuration.
     */
    LogConfig* getLogConfig() {return &m_logConfig;}

    /**
     * Sets the log configuration to the given module log configuration and sets the module name
     * This method should _only_ be called by the EventProcessor.
     *
     * @param moduleLogConfig Pointer to the logging configuration object of the module.
     *                        Set to NULL to use the global log configuration.
     * @param moduleName Name of the module.
     */
    void setModuleLogConfig(LogConfig* moduleLogConfig = 0, const std::string& moduleName = "-global-") {m_moduleLogConfig = moduleLogConfig; m_moduleName = moduleName; };

    /**
     * Add the per package log configuration.
     * Adds a new log configuration for the given package name.
     *
     * @param package The name of the package whose log configuration should be added.
     * @param logConfig The log configuration which should be assigned to the given package.
     */
    void addPackageLogConfig(const std::string& package, const LogConfig& logConfig) {m_packageLogConfigs[package] = logConfig; };

    /**
     * Get the log configuration for the package with the given name.
     * If no package specific configuration exists a new one is created.
     *
     * @param package The name of the package whose log configuration should be returned
     * @return The log configuration of the given package
     */
    LogConfig& getPackageLogConfig(const std::string& package) {return m_packageLogConfigs[package]; };

    /**
     * Returns true if the log level of the log system is greater or equal the given level.
     *
     * @param level The log level which should be compared with the log level of the log system
     * @param debugLevel The level for debug messages. Only used for the debug level.
     * @return True if the log level of the log system is greater or equal the given level.
     */
    bool isLevelEnabled(LogConfig::ELogLevel level, int debugLevel = 0, const std::string& package = "") const;

    /**
     * Sends a log message using the log connection object.
     * This method should _only_ be called by the logger macros.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(LogMessage message);

    /** Resets the message counter by setting all message counts to 0. */
    void resetMessageCounter();

    /**
     * Returns the number of logging calls per log level.
     *
     * @param logLevel The logging level which should be returned.
     * @return The number of message calls for the given log level.
     */
    int getMessageCounter(LogConfig::ELogLevel logLevel) const;

    /**
     * Returns the current log level used by the logging system.
     *
     * If the current log configuration is set to a module
     * log configuration, the log level of the module is returned.
     * Otherwise the global log level is returned.
     *
     * @return The current log level of the logging system.
     */
    LogConfig::ELogLevel getCurrentLogLevel() const;

    /**
     * Print error/warning summary at end of execution.
     *
     */
    void printErrorSummary();


  private:

    std::vector<LogConnectionBase*> m_logConnections;     /**< Stores the pointers to the log connection objects. */
    LogConfig m_logConfig;                                /**< The global log system configuration. */
    LogConfig* m_moduleLogConfig;                         /**< The current module log system configuration. */
    std::string m_moduleName;                             /**< The current module name. */
    std::map<std::string, LogConfig> m_packageLogConfigs; /**< Stores the log configuration objects for packages. */
    bool m_printErrorSummary;                             /**< Wether to re-print errors-warnings encountered during execution at the end. */
    std::vector<LogMessage> m_errorLog;                   /**< Save errors/warnings for summary. */

    int m_messageCounter[LogConfig::c_Default]; /**< Counts the number of messages sent per message level. */

    /** The constructor is hidden to avoid that someone creates an instance of this class. */
    LogSystem();

    /** Disable/Hide the copy constructor. */
    LogSystem(const LogSystem&);

    /** Disable/Hide the copy assignment operator. */
    LogSystem& operator=(const LogSystem&);

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

#endif /* LOGSYSTEM_H_ */
