/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  /// A mixin class to attach a name to an object. Based on class with same name in CDC package
  template <typename T>
  class Named {
  public:
    /// Constructor taking the name and the desired value.
    Named(std::string name, T value): m_name(name), m_value(value) {};

    /// get name of the object
    std::string getName() const {return m_name;}

    /// set name of the object
    void setName(std::string name) {m_name = name;}

    /// get the object
    T getValue() {return m_value;}

  protected:
    /// name associated with object m_value
    std::string m_name;
    /// object to be associated with name
    T m_value;
  };
}
