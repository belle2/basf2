/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2SOCKETEXCEPTION_H
#define B2SOCKETEXCEPTION_H

#include <string>

namespace Belle2 {

  /// @class B2SocketException
  /// @brief B2Socket exception handling class
  class B2SocketException {
  public:
    /// @brief B2SocketException class constructor
    /// @param s Description of the exception
    B2SocketException(std::string s) : m_s(s) {};
    /// @brief B2SocketException class destructor
    ~B2SocketException() {};

    /// @brief Exception describer
    /// @return Description of the exception
    std::string description() {
      return m_s;
    }

  private:
    std::string m_s;
  };
}

#endif
