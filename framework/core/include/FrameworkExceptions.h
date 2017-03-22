#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/format.hpp>

namespace Belle2 {

  /** Macro that defines an exception with the given message template.
   *
   *  Use the macro inside your class (header file) and specify a class name for your
   *  exception, plus a format string that contains a description of what went wrong:
      \code
      BELLE2_DEFINE_EXCEPTION(ModuleNotCreatedError, "Could not create module: %1%")
      \endcode
   *
   * When throwing, simply create an instance of this class, use operator<< to fill
   * any placeholders (%n%) specified, and pass this to the throw keyword:
      \code
      throw (ModuleNotCreatedError() << moduleName);
      \endcode
   *
   * Exceptions defined using this macro inherit from std::runtime_error, and can be queried
   * using their what() member.
   */

#define BELLE2_DEFINE_EXCEPTION(ClassName, Message) \
  class ClassName : public std::runtime_error { \
  public: \
    ClassName(): std::runtime_error(""), m_format(Message) { } \
    ~ClassName() noexcept {} \
    virtual const char * what() const noexcept override { \
      m_finalStr = m_format.str();\
      return m_finalStr.c_str();\
    }\
    template <class T> ClassName& operator<<(const T& param) {\
      m_format % param;\
      return *this;\
    }\
  private:\
    boost::format m_format;\
    mutable std::string m_finalStr;\
  };
}
