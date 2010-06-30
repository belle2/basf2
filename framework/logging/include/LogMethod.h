/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGMETHOD_H_
#define LOGMETHOD_H_

#include <logging/LogCommon.h>
#include <logging/LogSystem.h>

#include <string>

namespace Belle2 {

  //! The LogMethod class
  /*!
     This class allows the logging of the entering and leaving point of
     a method. Therefore an object of this class is created on the stack,
     which logs a message at the moment it is created and destroyed.
     In addition, it logs a message if there are any uncaught exceptions
     pending at the time the object is destroyed on the stack. This allows
     to identify methods which were terminated due to a thrown exception.
  */
  class LogMethod {

  public:

    //! The LogMethod constructor
    /*!
     \param package The package name where the message was sent from.
     \param function The function name where the message was sent from.
     \param file The file name where the message was sent from.
     \param line The line number in the source code where the message was sent from.
    */
    LogMethod(const std::string& package, const std::string& function, const std::string& file, unsigned int line);

    //! The LogMethod destructor
    ~LogMethod();


  protected:

  private:

    std::string m_package;  /*!< The package name where the message was sent from. */
    std::string m_function; /*!< The function name where the message was sent from. */
    std::string m_file;     /*!< The file name where the message was sent from. */
    unsigned int m_line;    /*!< The line number in the source code where the message was sent from. */

  };

} // end namespace Belle2

#endif /* LOGMETHOD_H_ */
