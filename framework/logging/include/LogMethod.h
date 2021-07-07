/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <string>

namespace Belle2 {

  /**
   * The LogMethod class.
   *
   * This class allows the logging of the entering and leaving point of
   * a method. Therefore an object of this class is created on the stack,
   * which logs a message at the moment it is created and destroyed.
   * In addition, it logs a message if there are any uncaught exceptions
   * pending at the time the object is destroyed on the stack. This allows
   * to identify methods which were terminated due to a thrown exception.
   */
  class LogMethod {

  public:

    /**
     * The LogMethod constructor.
     *
     * @param package The package name where the message was sent from.
     * @param function The function name where the message was sent from.
     * @param file The file name where the message was sent from.
     * @param line The line number in the source code where the message was sent from.
     */
    LogMethod(const char* package, std::string  function, std::string  file, unsigned int line);

    /** The LogMethod destructor. */
    ~LogMethod();


  protected:

  private:

    const char* m_package;  /**< The package name where the message was sent from. */
    std::string m_function; /**< The function name where the message was sent from. */
    std::string m_file;     /**< The file name where the message was sent from. */
    unsigned int m_line;    /**< The line number in the source code where the message was sent from. */

  };

} // end namespace Belle2


/**
 * \def B2METHOD()
 * scoped logging for entering/leaving methods.
 * Use this in a code block (curly braces) to get a B2INFO mesage
 * at the point where B2METHOD is used, and again when execution
 * leaves the current block.
 */
#ifdef LOG_NO_B2METHOD
#define B2METHOD() _B2_DO_NOTHING
#else
#define B2METHOD() Belle2::LogMethod logMethod(PACKAGENAME(), FUNCTIONNAME(), __FILE__, __LINE__)
#endif
