/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr, Thomas Hauth, Martin Ritter   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConfig.h>
#include <framework/logging/LogVariableStream.h>

#include <string>
#include <iosfwd>


namespace Belle2 {

  /**
   * The LogMessage class.
   *
   * This class encapsulates a logging message.
   */
  class LogMessage {
  public:

    /**
     * The LogMessage constructor.
     *
     * @param logLevel The log level of the message (e.g. debug, info, warning, error, fatal).
     * @param message The message string which should be send. This will be internally converted
     *                to a LogVariableStream with the text message and 0 variables.
     * @param package The package name where the message was sent from (can be NULL)
     * @param function The function name where the message was sent from.
     * @param file The file name where the message was sent from.
     * @param line The line number in the source code where the message was sent from.
     * @param debugLevel The debug level of the message if the logLevel is c_Debug
     */
    LogMessage(LogConfig::ELogLevel logLevel, const std::string& message, const char* package,
               std::string function, std::string file, unsigned int line, int debugLevel = 0);

    /**
     * The LogMessage constructor taking a LogVariableStream which can contains name/value pairs
     *
     * @param logLevel The log level of the message (e.g. debug, info, warning, error, fatal).
     * @param messageStream The LogVariableStream which should be send.
     * @param package The package name where the message was sent from (can be NULL)
     * @param function The function name where the message was sent from.
     * @param file The file name where the message was sent from.
     * @param line The line number in the source code where the message was sent from.
     * @param debugLevel The debug level of the message if the logLevel is c_Debug
     */
    LogMessage(LogConfig::ELogLevel logLevel, LogVariableStream&& messageStream, const char* package,
               std::string  function, std::string  file, unsigned int line, int debugLevel = 0);

    /**
     * Provide move constructor
     */
    LogMessage(LogMessage&&) = default;

    /**
     * Provide copy-constructor
     */
    LogMessage(LogMessage const& lm) = default;

    /**
     * Compares two messages.
     *
     * @return true if the message contents (including module, .cc line etc.) are equal.
     */
    bool operator==(const LogMessage& message) const;

    /** Check for inequality. */
    bool operator!=(const LogMessage& message) const { return !(*this == message); }

    /**
     * Custom assignment-operator, thanks to stringsream's incompetence ...
     */
    LogMessage& operator=(const LogMessage& lvs) = default;

    /**
     * Returns the log level of the message.
     *
     * @return Returns the log level of the message.
     */
    LogConfig::ELogLevel getLogLevel() const {return m_logLevel; }

    /**
     * Returns the package where the message was sent from.
     *
     * @return Returns the package where the message was sent from.
     */
    const std::string& getPackage() const {return m_package; }

    /**
     * Returns the message text.
     *
     * @return Returns the message text;
     */
    const std::string getMessage() const { return m_message.str(); }

    /**
     * Configure which information should be printed.
     */
    void setModule(const std::string& module) {m_module = module; }

    /**
     * Configure which information should be printed.
     */
    void setLogInfo(unsigned int logInfo) {m_logInfo = logInfo; }

    /**
     * Set the number of occurrences
     */
    void setCount(int count) {m_count = count; }

    /**
     * Generate output stream.
     *
     * @param out The output stream.
     */
    std::ostream& print(std::ostream& out) const;

    /** Return a json string for the log message.
     * The returned string will be a json object containing the full log message on one line
     * @param complete if true include all fields independent of the logInfo settings
     */
    std::string toJSON(bool complete) const;

    /** Return a reference to the log variables associated with this message */
    const std::vector<LogVar>& getLogVariables() const { return m_message.getVariables(); }

    /** Helper struct to hash and compare messages only by log level and message content.
     *
     * This explicitly ignores other information like log variables and module/package/file/line
     * information to just combine messages which will look similar to the user.
     */
    struct TextHasher {
      /** Return a hash for the given log message just based on the log level and the message */
      std::size_t operator()(const LogMessage& msg) const
      {
        return msg.m_logLevel ^ std::hash<std::string>()(msg.m_message.getMessage());
      }
      /** Check if log level and message text are identical */
      bool operator()(const LogMessage& lhs, const LogMessage& rhs) const
      {
        return (lhs.m_logLevel == rhs.m_logLevel) &&
               (lhs.m_message.getMessage() == rhs.m_message.getMessage());
      }
    };
  private:

    LogConfig::ELogLevel m_logLevel; /**< The log level of the message. */
    LogVariableStream m_message;     /**< The message stream which should be sent. */
    std::string m_module;     /**< The module name where the message was sent from. */
    std::string m_package;    /**< The package name where the message was sent from. */
    std::string m_function;   /**< The function name where the message was sent from. */
    std::string m_file;       /**< The file name where the message was sent from. */
    unsigned int m_line;      /**< The line number in the source code where the message was sent from. */
    int m_debugLevel;         /**< The debug level for messages with level=c_Debug */

    unsigned int m_logInfo;   /**< kind of information to show (ORed combination of LogConfig::ELogInfo flags). */
    int m_count;              /**< Number of occurrences of the same message */
  };
} // end namespace Belle2

/**
 * Operator to print a log message.
 */
extern std::ostream& operator<< (std::ostream& out, const Belle2::LogMessage& logMessage);
