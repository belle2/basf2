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
     * @param message The message string which should be send.
     * @param package The package name where the message was sent from (can be NULL)
     * @param function The function name where the message was sent from.
     * @param file The file name where the message was sent from.
     * @param line The line number in the source code where the message was sent from.
     */
    LogMessage(LogConfig::ELogLevel logLevel, const std::string& message, const char* package,
               const std::string& function, const std::string& file, unsigned int line, int debugLevel = 0);

    /**
     * Compares two messages.
     *
     * @return true if the message contents (including module, .cc line etc.) are equal.
     */
    bool operator==(const LogMessage& message) const;

    /** Check for inequality. */
    bool operator!=(const LogMessage& message) const { return !(*this == message); }

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
    const std::string& getMessage() const { return m_message; }

    /**
     * Configure which information should be printed.
     */
    void setModule(const std::string& module) {m_module = module; }

    /**
     * Configure which information should be printed.
     */
    void setLogInfo(unsigned int logInfo) {m_logInfo = logInfo; }

    /**
     * Generate output stream.
     *
     * @param out The output stream.
     */
    std::ostream& print(std::ostream& out) const;


  private:

    LogConfig::ELogLevel m_logLevel;  /**< The log level of the message. */
    std::string m_message;    /**< The message string which should be sent. */
    std::string m_module;     /**< The module name where the message was sent from. */
    std::string m_package;    /**< The package name where the message was sent from. */
    std::string m_function;   /**< The function name where the message was sent from. */
    std::string m_file;       /**< The file name where the message was sent from. */
    unsigned int m_line;      /**< The line number in the source code where the message was sent from. */
    int m_debugLevel;         /**< The debug level for messages with level=c_Debug */

    unsigned int m_logInfo;   /**< kind of information to show (ORed combination of LogConfig::ELogInfo flags). */


    friend size_t hash(const LogMessage& msg);
  };
  /** Calculate hash of this message, for unordered_map etc. */
  size_t hash(const LogMessage& msg);

} // end namespace Belle2

/**
 * Operator to print a log message.
 */
extern std::ostream& operator<< (std::ostream& out, const Belle2::LogMessage& logMessage);
