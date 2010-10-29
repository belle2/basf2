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

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <string>

namespace Belle2 {

  //Macro that defines an exception
#define BELLE2_DEFINE_EXCEPTION(ClassName, Message) \
  class ClassName : public std::runtime_error { \
  public: \
    ClassName(): std::runtime_error(Message), m_arg1(""), m_arg2(""), m_arg3(""), m_arg4("") { } \
    ClassName(std::string arg1, std::string arg2="", std::string arg3="", std::string arg4=""): std::runtime_error(Message),m_arg1(arg1),m_arg2(arg2),m_arg3(arg3),m_arg4(arg4) { } \
    ~ClassName() throw() {} \
    virtual const char * what() const throw() { \
      if (m_arg1.empty()) return Message;\
      if (m_arg2.empty()) return (boost::format(Message) %m_arg1).str().c_str();\
      if (m_arg3.empty()) return (boost::format(Message) %m_arg1 %m_arg2).str().c_str();\
      if (m_arg4.empty()) return (boost::format(Message) %m_arg1 %m_arg2 %m_arg3).str().c_str();\
      return (boost::format(Message) %m_arg1 %m_arg2 %m_arg3 %m_arg4).str().c_str();\
    }\
  private:\
    std::string m_arg1;\
    std::string m_arg2;\
    std::string m_arg3;\
    std::string m_arg4;\
  };

#define TO_STR(value) boost::lexical_cast<std::string>(value)

}

#endif /* FRAMEWORKEXCEPTIONS_H_ */
