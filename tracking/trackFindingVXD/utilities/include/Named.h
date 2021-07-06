/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  /// A mixin class to attach a name to an object. Based on class with same name in CDC package
  template <typename T>
  class Named {
  public:
    /// Constructor taking the name and the desired value.
    Named(const std::string& name, T value):
      m_name(name), m_value(value)
    {};

    /// get name of the object
    std::string getName() const {return m_name;}

    /// set name of the object
    void setName(const std::string& name)
    {
      m_name = name;
    }

    /// get the object
    T getValue()
    {
      return m_value;
    }

  protected:
    /// name associated with object m_value
    std::string m_name;
    /// object to be associated with name
    T m_value;
  };
}
