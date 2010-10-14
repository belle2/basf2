/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGSYSTEM_H_
#define LOGSYSTEM_H_

#include <framework/logging/LogCommon.h>
#include <framework/logging/LogConnectionAbs.h>
#include <framework/logging/LogConnectionIOStream.h>

#include <string>


namespace Belle2 {

  /*! Class for logging debug, info and error messages */
  /*! For log messages the following preprocessor macros should be used:
        DEBUG   for code development
        INFO    for informational messages, e.g. about opened files, processed events, etc.
        WARNING for potential problems that the user should pay attention to
        ERROR   for things that went wrong and have to be fixed
        FATAL   for situations were the program execution can not be continued

      By default the Logger sends messages to std::cout.

      Using preprocessor macros has two advantages. First the log statements can be removed
      from the code completely by redefining the macros. Second they allow to add information
      about the location (package, function, file, line).

      This class is designed as a singleton.
  */
  class LogSystem {

  public:

    /*! Static method to get a reference to the LogSystem instance. */
    /*!
      \return A reference to an instance of this class.
    */
    static LogSystem& Instance();

    /*! Sets the log connection object which is used to the send the logging messages. */
    /*!
     Please note: The LogSystem class takes ownership of the log connection object.

     \param logConnection Pointer to the logging connection object. The LogSystem takes ownership of this object.
    */
    void setLogConnection(LogConnectionAbs* logConnection);

    /*! Sets the log level (messaging intensity). */
    /*! \param level The level of the message intensity (e.g. debug, info, warning, error, fatal).
        \sa enum ELogLevel
    */
    void setLogLevel(LogCommon::ELogLevel level) {m_logLevel = level;}

    /*! Returns the log level (messaging intensity). */
    /*! \return The current log level of the log system.
        \sa enum ELogLevel
    */
    LogCommon::ELogLevel getLogLevel() {return m_logLevel;}

    /*! Returns true if the log level of the log system is greater or equal the given level. */
    /*!
        \param level The log level which should be compared with the log level of the log system
        \param debugLevel The level for debug messages. Only used for the debug level.
        \return True if the log level of the log system is greater or equal the given level.
    */
    bool isLevelEnabled(LogCommon::ELogLevel level, int debugLevel = 0);

    /*! Sets the messaging intensity for debug messages. */
    /*! The debug level is only considered if the messaging level is at least set to DEBUG.
        The meaning of a certain level depends on the usage by the developer.
    */
    void setDebugLevel(int level) {m_debugLevel = level;}

    /*! Returns the current debug messaging level. */
    /*! \return The debug level as it was set by the developer.
    */
    int getDebugLevel() {return m_debugLevel;}

    /*! Sends a log message using the log connection object. */
    /*!
     \param logLevel The log level of the message (e.g. debug, info, warning, error, fatal).
     \param message The message string which should be send.
     \param package The package name where the message was sent from.
     \param function The function name where the message was sent from.
     \param file The file name where the message was sent from.
     \param line The line number in the source code where the message was sent from.
     \param sendLocationInfo If true, the location info (package, function, file, line) is sent
     \return Returns true if the message could be send.
    */
    bool sendMessage(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                     const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo = true);

    /*! Sends a log message using the log connection object and immediately after having send the message abort the program. */
    /*!
     This method is used by the fatal message, which forces the termination of the program execution.
     The currently set debug level is NOT checked.

     \param logLevel The log level of the message (e.g. debug, info, warning, error, fatal).
     \param message The message string which should be send.
     \param package The package name where the message was sent from.
     \param function The function name where the message was sent from.
     \param file The file name where the message was sent from.
     \param line The line number in the source code where the message was sent from.
     \param sendLocationInfo If true, the location info (package, function, file, line) is sent
    */
    void sendMessageForceAbort(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                               const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo = true);

    /*! Resets the message counter by setting all message counts to 0. */
    void resetMessageCounter();

    /*! Returns the number of logging calls per log level. */
    /*!
      \param logLevel The logging level which should be returned.
      \return The number of message calls for the given log level.
     */
    int getMessageCounter(LogCommon::ELogLevel logLevel);


  private:

    LogConnectionAbs* m_logConnection; /*!< Stores the pointer to the log connection object. */
    LogCommon::ELogLevel m_logLevel;   /*!< the current log level of the log system. */
    int m_debugLevel;                  /*!< the current debug messaging level of the log system. */

    int* m_messageCounter;             /*!< Counts the number of messages sent per message level. */

    /*! The constructor is hidden to avoid that someone creates an instance of this class. */
    LogSystem();

    /*! Disable/Hide the copy constructor */
    LogSystem(const LogSystem&);

    /*! Disable/Hide the copy assignment operator */
    LogSystem& operator=(const LogSystem&);

    /*! The LogSystem destructor */
    ~LogSystem();

    /*! Increases the counter for the called message level by one. */
    /*!
      \param logLevel The logging level which should be increased by one.
     */
    void incMessageCounter(LogCommon::ELogLevel logLevel);

    static LogSystem* m_instance; /*!< Pointer that saves the instance of this class. */

    /*! Destroyer class to delete the instance of the LogSystem class when the program terminates. */
    class SingletonDestroyer {
    public: ~SingletonDestroyer() {
        if (LogSystem::m_instance != NULL) delete LogSystem::m_instance;
      }
    };

    friend class SingletonDestroyer;

  };

} //end of namespace Belle2

#endif /* LOGSYSTEM_H_ */
