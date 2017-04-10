/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  template <typename T>
  class Named {
  public:
    Named(std::string name, T value): m_name(name), m_value(value) {};

    bool operator== (const std::string& other) const
    {
      return (m_name == other);
    }
    bool operator!=(const std::string& other) const
    {
      return !(*this == other);
    }

    Named<T>& operator= (T value)
    {
      m_value = value;
      return *this;
    }

    std::string getName() const {return m_name;}
    void setName(std::string name) {m_name = name;}

    T* getValuePtr() {return &m_value;}

  protected:
    std::string m_name;
    T m_value;
  };
}
