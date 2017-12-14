#ifndef _Belle2_MonXMLElement_h
#define _Belle2_MonXMLElement_h

#include <string>

namespace Belle2 {

  class MonXMLElement {

  public:
    MonXMLElement(const std::string& name = "") throw() : m_name(name) {}
    virtual ~MonXMLElement() throw() {}

  public:
    void setName(const std::string& name) throw() { m_name = name; }
    const std::string& getName() const throw() { return m_name; }
    virtual std::string getTag() const throw() = 0;
    virtual std::string toXML() const throw() = 0;

  protected:
    std::string m_name;

  };

};

#endif
