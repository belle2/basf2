#ifndef _Belle2_XMLElement_h
#define _Belle2_XMLElement_h

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class XMLElement {

  public:
    typedef std::map<std::string, std::string> AttributeList;
    typedef std::vector<XMLElement*> ElementList;

  public:
    XMLElement(const std::string tag = "")
      : _tag(tag) {}
    ~XMLElement() throw() {
      for (size_t i = 0; i < _el_v.size(); i++) {
        delete _el_v[i];
      }
    }

  public:
    void setTag(const std::string& tag) { _tag = tag; }
    const std::string& getTag() const { return _tag; }
    bool hasAttribute(const std::string& name) {
      return (_attr_m.find(name) != _attr_m.end());
    }
    void addAttribute(const std::string& name, const std::string& value) {
      if (hasAttribute(name)) _attr_m[name] = value;
      else _attr_m.insert(AttributeList::value_type(name, value));
    }
    void setAttribute(const std::string& name, const std::string& value) {
      if (hasAttribute(name)) _attr_m[name] = value;
      else addAttribute(name, value);
    }
    const std::string& getAttribute(const std::string& name) {
      return operator[](name);
    }
    std::string& operator[](const std::string& name) {
      if (!hasAttribute(name)) addAttribute(name, "");
      return _attr_m[name];
    }
    AttributeList& getAttributes() { return _attr_m; }
    void addElement(XMLElement* el) { _el_v.push_back(el); }
    ElementList& getElements() { return _el_v; }

  private:
    std::string _tag;
    AttributeList _attr_m;
    ElementList _el_v;

  };

}

#endif
