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


//(ifndef required for the rootcint dictionary building)
#ifndef __CINT__

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
    ClassName(): std::runtime_error(Message), m_format(Message) { } \
    ~ClassName() throw() {} \
    virtual const char * what() const throw() { \
      return m_format.str().c_str();\
    }\
    template <class T> ClassName& operator<<(const T& param) {\
      m_format % param;\
      return *this;\
    }\
  private:\
    boost::format m_format;\
  };
}

#else
/** Macro that defines an exception with the given message template. */
#define BELLE2_DEFINE_EXCEPTION(ClassName, Message)
#endif
