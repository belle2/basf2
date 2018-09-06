/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <ios>

class LogVar {
public:
  template<class TVarType>
  LogVar(std::string name, TVarType v) :
    m_name(name),
    m_value(boost::lexical_cast<std::string>(v))
  {
  }

  std::string getValue() const
  {
    return m_value;
  }

  std::string getName() const
  {
    return m_name;
  }

private:
  const std::string m_name;
  const std::string m_value;
};

class LogVariableStream { /*: public std::stringstream*/
public:
  LogVariableStream() = default;

  LogVariableStream(std::string withText,
                    std::vector<LogVar> variables = { }) :
    m_variables(variables)
  {
    m_stringStream << withText;
  }

  template<class TText>
  LogVariableStream& operator<<(TText const& text)
  {
    m_stringStream << text;

    return *this;
  }
  /*
    LogVariable

    template<class TText>
    LogVariableStream& operator<<(TText & text) {
      m_stringStream << text;

      return *this;
    }*/

  typedef std::basic_ios<std::stringstream::char_type, std::stringstream::traits_type >__ios_type;
  typedef std::basic_ostream<std::stringstream::char_type, std::stringstream::traits_type > __basic_ostream_type;


  LogVariableStream& operator<<(__basic_ostream_type & (*__pf)(__basic_ostream_type&))
  {
    //m_variables.push_back(var);
    __pf(m_stringStream);

    return *this;
  }
  /*
    LogVariableStream& operator<<(__ios_type& (*__pf)(__ios_type&) ) {
      //m_variables.push_back(var);

      return *this;
    }

    LogVariableStream& operator<<(std::ios_base& (*__pf)(std::ios_base&) ) {
      //m_variables.push_back(var);

      //__pf(*this);

      return *this;
    }*/


  LogVariableStream& operator<<(LogVar const& var)
  {
    m_variables.push_back(var);

    return *this;
  }
  /*
    LogVariableStream& operator<<(LogVar const& var) {
      m_variables.push_back(var);

      return *this;
    }*/

  std::string str() const
  {
    // little optimization, so we don't need to copy the whole string
    // in the cases where there are no variables ...
    if (m_variables.size() == 0)
      return m_stringStream.str();

    std::stringstream s;
    s <<  m_stringStream.str();
    for (auto const& v : m_variables) {
      s << std::endl << "\t" << v.getName() << " = " << v.getValue();
    }
    return s.str();
  }

private:
  std::stringstream m_stringStream;
  std::vector<LogVar> m_variables;
};
