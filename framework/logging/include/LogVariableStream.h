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
#include <vector>
#include <string>
#include <map>
#include <type_traits>
#include <boost/lexical_cast.hpp>

/**
 * Class to store variables with their name which were sent to
 * the logging service.
 */
class LogVar {
public:
  /**
   * Constructor which accepts any type as value and relies on boost lexical cast.
   * @param name String to identify the logged value
   * @param v Value to log, can be of any type which boost::lexical_cast is able to
   *          convert to a string.
   */
  template<class TVarType>
  LogVar(const std::string& name, const TVarType& v) :
    m_name(name),
    m_value(boost::lexical_cast<std::string>(v))
  {
  }

  /**
   * Returns the value stored for this variable.
   */
  std::string getValue() const
  {
    return m_value;
  }

  /**
   * Returns the name stored for this variable.
   */
  std::string getName() const
  {
    return m_name;
  }

  /**
   * Custom comparison operator
   */
  bool operator==(const LogVar& lv) const
  {
    return (lv.m_name == this->m_name) && (lv.m_value == this->m_value);
  }

private:
  /** Stores the name of the variable. Best is a short string to describe which
   * variable is presented, like "node number" etc. */
  std::string m_name;

  /** String conversion of
   *  the value. */
  std::string m_value;
};

/**
 * Specialized implementation of an ostream-like class where the << operator can be used to insert values.
 * In addition to the regular ostream usage, this class also accepts the LogVar class, which contains
 * the name of a variable and its value. If string part and variable part of a log message are separated in
 * that manner, it is much easier to filter and aggregate messages.
 *
 * Here is an example on the usage:
 *           LogVariableStream lvs;
 *           lvs << "Inconsistent data size between COPPER and CDC FEE."
 *               << LogVar("data length", dataLength) << LogVar("nWord", nWord)
 *               << LogVar("Node ID", iNode) << LogVar("Finness ID", iFiness));
 *           std::string lvsAsString = lvs.str();
 *
 */
class LogVariableStream {
public:

  /** basic_ofstream which is used with ostream's utility functions */
  typedef std::basic_ostream<std::stringstream::char_type, std::stringstream::traits_type > __basic_ostream_type;

  /** Default constructor with empty text and no variables */
  LogVariableStream() = default;

  /**
   * Provide default move constructor
   */
  LogVariableStream(LogVariableStream&&) = default;

  /**
   * Implement custom copy-constructor, because stringstream's one is deleted.
   */
  LogVariableStream(const LogVariableStream& other) :  m_variables(other.m_variables)
  {
    // copy manually because stringstream has no copy-constructor
    m_stringStream << other.m_stringStream.str();
  }

  /** Constructor which sets an initial text for this stream
   * @param text Initial text
   * */
  LogVariableStream(std::string const& text, std::map<std::string, std::string> variables = {})
  {
    m_stringStream << text;
    for (auto const& kv : variables) {
      m_variables.emplace_back(LogVar(kv.first, kv.second));
    }
  }

  /**
   * operator override for ostream modifier functions like std::endl who are directly
   * applied to the underlying string stream.
   */
  LogVariableStream& operator<<(__basic_ostream_type & (*__pf)(__basic_ostream_type&))
  {
    // execute provided function on the string stream
    __pf(m_stringStream);
    return *this;
  }

  /**
   * Operator override which stores the LogVar information instead of putting it
   * directly in the sstream
   */
  LogVariableStream& operator<<(LogVar const& var)
  {
    m_variables.push_back(var);
    return *this;
  }

  /**
   * Templated operator which will be used for all non-fundamental types. This types can be accepted via
   * const& and need no copy.
   */
  template<class TText>
  typename std::enable_if<not std::is_fundamental<TText>::value, LogVariableStream&>::type operator<<(TText const& text)
  {
    this->m_stringStream << text;
    return *this;
  }

  /**
   * Templated operator which will be used for POD types (especially integers) and uses by-value. For cases where constants are
   * declared "static const int Name = 23;" in header files but the .cc file contains no definition. In these cases, by-ref
   * cannot be used because no memory location exists to get the reference.
   */
  template<class PODTYPE>
  typename std::enable_if<std::is_fundamental<PODTYPE>::value, LogVariableStream&>::type operator<<(PODTYPE pod)
  {
    this->m_stringStream << pod;
    return *this;
  }

  /**
   * Custom comparison operator
   */
  bool operator==(const LogVariableStream& lvs) const
  {
    return (lvs.m_variables == this->m_variables) && (lvs.m_stringStream.str() == this->m_stringStream.str());
  }

  /**
   * Custom assignment-operator, thanks to stringsream's non-copy policy
   */
  LogVariableStream& operator=(const LogVariableStream& lvs)
  {
    this->m_stringStream = std::stringstream();
    this->m_stringStream << lvs.m_stringStream.str();
    this->m_variables = lvs.m_variables;
    return *this;
  }

  /**
   * Return the content of the stream as string. First the stringstream part
   * and then a list of the variables
   */
  std::string str() const
  {
    // little optimization, so we don't need to copy the whole string
    // in the cases where there are no variables ...
    if (m_variables.size() == 0) {
      return m_stringStream.str();
    }

    std::stringstream s;
    // put the string first
    s <<  m_stringStream.str();
    for (auto const& v : m_variables) {
      s << std::endl << "\t" << v.getName() << " = " << v.getValue();
    }
    return s.str();
  }

private:

  /** All non-LogVar items are directly forwarded to this stringstream */
  std::stringstream m_stringStream;

  /** List of LogVars which were accepted so far */
  std::vector<LogVar> m_variables;
};
