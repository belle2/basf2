/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Enum_hh
#define _Belle2_Enum_hh

#include <string>

namespace Belle2 {

  class Enum {

  public:
    const static Enum UNKNOWN;

  public:
    ~Enum() {}

  protected:
    Enum(int id, const std::string& label)
      : m_id(id), m_label(label) {}
    Enum(const Enum& e) :
      m_id(e.m_id), m_label(e.m_label) {}
    Enum() : m_id(UNKNOWN.m_id),
      m_label(UNKNOWN.m_label) {}

  public:
    int getId() const { return m_id; }
    const char* getLabel() const { return m_label.c_str(); }
    const std::string& getLabelString() const { return m_label; }

  public:
    bool operator==(const Enum& e) const;
    bool operator!=(const Enum& e) const;
    const Enum& operator=(const Enum& e);
    bool operator==(const std::string& label) const;
    bool operator!=(const std::string& label) const;

  private:
    int m_id;
    std::string m_label;

  };

  inline bool Enum::operator==(const Enum& e) const
  {
    return (m_id == e.m_id);
  }
  inline bool Enum::operator!=(const Enum& e) const
  {
    return (m_id != e.m_id);
  }
  inline const Enum& Enum::operator=(const Enum& e)
  {
    m_id = e.m_id;
    m_label = e.m_label;
    return *this;
  }
  inline bool Enum::operator==(const std::string& label) const
  {
    return m_label == label;
  }
  inline bool Enum::operator!=(const std::string& label) const
  {
    return m_label != label;
  }

}

#endif
