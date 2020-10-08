/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2020 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConfig.h>
#include <framework/logging/LogMessage.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>


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
     * @param message The log message object. The message might
     *                be moved into internal storage and is not valid
     *                any more after the method call.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(LogMessage&& message);

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
    LogConfig::ELogLevel getCurrentLogLevel(const char* package = nullptr) const { return getCurrentLogConfig(package).getLogLevel(); }

    /**
     * Returns the current debug level used by the logging system.
     *
     * \sa getCurrentLogConfig()
     */
    int getCurrentDebugLevel(const char* package = nullptr) const { return getCurrentLogConfig(package).getDebugLevel(); }

    /**
     * Get maximum number of repetitions before silencing "identical" log messages
     *
     * Identity only checks for log level and message content (excluding variables)
     */
    unsigned getMaxMessageRepetitions() const { return m_maxErrorRepetition; }

    /**
     * Set maximum number of repetitions before silencing "identical" log messages
     *
     * Identity only checks for log level and message content (excluding variables)
     */
    void setMaxMessageRepetitions(unsigned repetitions) { m_maxErrorRepetition = repetitions; }

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

    /**
     * Enable debug output.
     */
    static void enableDebug() {s_debugEnabled = true;}

    /**
     * Is debug output enabled?
     */
    inline static bool debugEnabled() {return s_debugEnabled;}

  private:
    /** Stores the pointers to the log connection objects. (owned by us) */
    std::vector<LogConnectionBase*> m_logConnections;
    /** The global log system configuration. */
    LogConfig m_logConfig;
    /** log config of current module */
    const LogConfig* m_moduleLogConfig;
    /** The current module name. */
    std::string m_moduleName;
    /** Stores the log configuration objects for packages. */
    std::map<std::string, LogConfig> m_packageLogConfigs;
    /** Wether to re-print errors-warnings encountered during execution at the end. */
    bool m_printErrorSummary;
    /** Count of previous log messages for the summary and to suppress repetitive messages */
    std::unordered_map<LogMessage, int, LogMessage::TextHasher, LogMessage::TextHasher> m_messageLog{100};
    /** Maximum number to show the same message. If zero we don't suppress repeated messages */
    unsigned int m_maxErrorRepetition{0};
    /** The amount of messages we have suppressed so far just to get an indication we print this from time to time */
    unsigned int m_suppressedMessages{0};
    /** Counts the number of messages sent per message level. */
    int m_messageCounter[LogConfig::c_Default];
    /** Global flag for fast checking if debug output is enabled */
    static bool s_debugEnabled;

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

    /** Do nothing else than to send the message to all connected connections. No bookkeeping, just and and be done */
    bool deliverMessageToConnections(const LogMessage& msg);

    /** Send a custom message which looks like a log message but should not be
     * counted as such.
     *
     * This is used in a few places to emit descriptive messages related to
     * other messages without danger of being suppressed
     */
    void showText(LogConfig::ELogLevel level, const std::string& txt, int info = LogConfig::c_Message);
  };

  inline const LogConfig& LogSystem::getCurrentLogConfig(const char* package) const
  {
    //module specific config?
    if (m_moduleLogConfig && (m_moduleLogConfig->getLogLevel() != LogConfig::c_Default)) {
      return *m_moduleLogConfig;
    }
    //package specific config?
    if (package && !m_packageLogConfigs.empty()) {
      // cppcheck-suppress stlIfFind ; cppcheck doesn't like scoped variables in if statements
      if (auto it = m_packageLogConfigs.find(package); it != m_packageLogConfigs.end()) {
        const LogConfig& logConfig = it->second;
        if (logConfig.getLogLevel() != LogConfig::c_Default)
          return logConfig;
      }
    }
    //global config
    return m_logConfig;
  }

  inline bool LogSystem::isLevelEnabled(LogConfig::ELogLevel level, int debugLevel, const char* package) const
  {
    const LogConfig& config = getCurrentLogConfig(package);
    const LogConfig::ELogLevel logLevelLimit = config.getLogLevel();
    const int debugLevelLimit = config.getDebugLevel();
    return logLevelLimit <= level && (level != LogConfig::c_Debug || debugLevelLimit >= debugLevel);
  }

} //end of namespace Belle2
