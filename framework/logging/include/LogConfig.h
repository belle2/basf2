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

namespace Belle2 {

  /**
   * The LogConfig class.
   *
   * This class provides configuration settings for the logging system.
   */
  class LogConfig {
  public:

    /** Definition of the supported log levels. In ascending order ! */
    enum ELogLevel { c_Debug,   /**< Debug: for code development.  */
                     c_Info,    /**< Info: for informational messages, e.g. about opened files, processed events, etc. */
                     c_Result,  /**< Result: for informational summary messages, e.g. about number of selected events */
                     c_Warning, /**< Warning: for potential problems that the user should pay attention to. */
                     c_Error,   /**< Error: for things that went wrong and have to be fixed. */
                     c_Fatal,   /**< Fatal: for situations were the program execution can not be continued. */
                     c_Default  /**< Default: use globally configured log level.  */
                   };

    /** Definition of the supported log message information. */
    enum ELogInfo { c_Level = 0x01,    /**< Log level of the message.  */
                    c_Message = 0x02,  /**< Log message text.  */
                    c_Module = 0x04,   /**< Module in which the message was emitted. */
                    c_Package = 0x08,  /**< Package in which the message was emitted. */
                    c_Function = 0x10, /**< Function in which the message was emitted. */
                    c_File = 0x20,     /**< Source file in which the message was emitted. */
                    c_Line = 0x40,     /**< Line in source file in which the message was emitted. */
                    c_Timestamp = 0x80 /**< Time at which the message was emitted. */
                  };

    static const int c_DefaultDebugLevel = 10;  /**< Default debug level. */

    /**
     * The LogConfig constructor.
     *
     * @param logLevel The log level.
     * @param debugLevel The debug level.
     */
    LogConfig(ELogLevel logLevel = c_Default, int debugLevel = c_DefaultDebugLevel);

    /**
     * Converts a log level type to a string.
     *
     * @param logLevelType The type of the log level.
     * @return The string representation of the log level type.
     */
    static const char* logLevelToString(ELogLevel logLevelType);

    /**
     * Configure the log level.
     *
     * @param logLevel The log level.
     */
    void setLogLevel(ELogLevel logLevel);

    /**
     * Returns the configured log level.
     *
     * @return Returns the configured log level.
     */
    ELogLevel getLogLevel() const {return m_logLevel; }

    /**
     * Configure the debug messaging level.
     *
     * @param debugLevel The debug level.
     */
    void setDebugLevel(int debugLevel) {m_debugLevel = debugLevel; }

    /**
     * Returns the configured debug messaging level.
     *
     * @return Returns the configured debug messaging level.
     */
    int getDebugLevel() const {return m_debugLevel; }

    /**
     * Configure the abort level.
     *
     * @param abortLevel The abort log level.
     */
    void setAbortLevel(ELogLevel abortLevel) {m_abortLevel = abortLevel; }

    /**
     * Returns the configured abort level.
     *
     * @return Returns the configured abort level.
     */
    ELogLevel getAbortLevel() const {return m_abortLevel; }

    /**
     * Configure the printed log information for the given level.
     *
     * @param logLevel The log level.
     * @param logInfo The log information that should be printed.
     */
    void setLogInfo(ELogLevel logLevel, unsigned int logInfo) {m_logInfo[logLevel] = logInfo; }

    /**
     * Returns the configured log information for the given level.
     *
     * @return Returns the configured log information.
     */
    unsigned int getLogInfo(ELogLevel logLevel) const {return m_logInfo[logLevel]; }


  private:

    ELogLevel m_logLevel;               /**< The log messaging level. Defined as int for the parameter handling. */
    int m_debugLevel;                   /**< The debug messaging level. */
    ELogLevel m_abortLevel;             /**< The log level at which the execution should be stopped. */
    unsigned int
    m_logInfo[c_Default];  /**< The kind of printed information per log level. (ORed combination of LogConfig::ELogInfo flags). */

  };

} // end namespace Belle2
