/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FRAMEWORKEXCEPTIONS_H_
#define FRAMEWORKEXCEPTIONS_H_

//(ifndef required for the rootcint dictionary building)
#ifndef __CINT__

#include <boost/format.hpp>

namespace Belle2 {

  /** Macro that defines an exception with the given message template. */
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


#endif /* FRAMEWORKEXCEPTIONS_H_ */
