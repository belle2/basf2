#ifndef _Belle2_Enum_hh
#define _Belle2_Enum_hh

#include <string>

namespace Belle2 {

  class Enum {

  public:
    const static Enum UNKNOWN;

  public:
    ~Enum() throw() {}

  protected:
    Enum(int id, const std::string& label)
    throw() : m_id(id), m_label(label) {}
    Enum(const Enum& e) throw() :
      m_id(e.m_id), m_label(e.m_label) {}
    Enum() throw() : m_id(UNKNOWN.m_id),
      m_label(UNKNOWN.m_label) {}

  public:
    int getId() const throw() { return m_id; }
    const char* getLabel() const throw() { return m_label.c_str(); }
    const std::string& getLabelString() const throw() { return m_label; }

  public:
    bool operator==(const Enum& e) const throw();
    bool operator!=(const Enum& e) const throw();
    const Enum& operator=(const Enum& e) throw();

  private:
    int m_id;
    std::string m_label;

  };

  inline bool Enum::operator==(const Enum& e) const throw()
  {
    return (m_id == e.m_id);
  }
  inline bool Enum::operator!=(const Enum& e) const throw()
  {
    return (m_id != e.m_id);
  }
  inline const Enum& Enum::operator=(const Enum& e) throw()
  {
    m_id = e.m_id;
    m_label = e.m_label;
    return *this;
  }

}

#endif
