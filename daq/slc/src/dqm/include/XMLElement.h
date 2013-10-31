#ifndef _Belle2_XMLElement_hh
#define _Belle2_XMLElement_hh

#include <string>

namespace Belle2 {

  class XMLElement {

  public:
    XMLElement(const std::string& name = "") throw() : _name(name) {}
    virtual ~XMLElement() throw() {}

  public:
    void setName(const std::string& name) throw() { _name = name; }
    const std::string& getName() const throw() { return _name; }
    virtual std::string getTag() const throw() = 0;
    virtual std::string toXML() const throw() = 0;

  protected:
    std::string _name;

  };

};

#endif
