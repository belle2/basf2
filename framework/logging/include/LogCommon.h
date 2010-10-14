/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGCOMMON_H_
#define LOGCOMMON_H_

#include <string>

namespace Belle2 {

  /*!  The LogCommon class. */
  /*!
    This class provides commonly used methods for the logging library.
  */
  class LogCommon {
  public:

    /*! Definition of the supported log levels. In ascending order ! */
    enum ELogLevel { c_Debug,   /*!< Debug: for code development.  */
                     c_Info,    /*!< Info: for informational messages, e.g. about opened files, processed events, etc. */
                     c_Warning, /*!< Warning: for potential problems that the user should pay attention to. */
                     c_Error,   /*!< Error: for things that went wrong and have to be fixed. */
                     c_Fatal    /*!< Fatal: for situations were the program execution can not be continued. */
                   };

    /*! The number of log levels. Make sure this number corresponds to the number of entries in ELogLevel. */
    static const int ELogLevelCount = 5;

    /*! Converts a log level type to a string. */
    /*!
      \param logLevelType The type of the log level.
      \return The string representation of the log level type.
    */
    static std::string logLevelToString(ELogLevel logLevelType) {
      std::string result;

      switch (logLevelType) {
        case c_Debug:
          result = "DEBUG";
          break;
        case c_Info:
          result = "INFO";
          break;
        case c_Warning:
          result = "WARNING";
          break;
        case c_Error:
          result = "ERROR";
          break;
        case c_Fatal:
          result = "FATAL";
          break;
        default:
          result = "unknown";
      }

      return result;
    }
  };

} // end namespace Belle2

#endif /* LOGCOMMON_H_ */
