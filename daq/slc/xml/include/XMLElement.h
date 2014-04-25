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
      : m_tag(tag) {}
    ~XMLElement() throw() {
      for (size_t i = 0; i < m_el_v.size(); i++) {
        delete m_el_v[i];
      }
    }

  public:
    void setTag(const std::string& tag) { m_tag = tag; }
    const std::string& getTag() const { return m_tag; }
    bool hasAttribute(const std::string& name) {
      return (m_attr_m.find(name) != m_attr_m.end());
    }
    void addAttribute(const std::string& name, const std::string& value) {
      if (hasAttribute(name)) m_attr_m[name] = value;
      else m_attr_m.insert(AttributeList::value_type(name, value));
    }
    void setAttribute(const std::string& name, const std::string& value) {
      if (hasAttribute(name)) m_attr_m[name] = value;
      else addAttribute(name, value);
    }
    const std::string& getAttribute(const std::string& name) {
      return operator[](name);
    }
    std::string& operator[](const std::string& name) {
      if (!hasAttribute(name)) addAttribute(name, "");
      return m_attr_m[name];
    }
    AttributeList& getAttributes() { return m_attr_m; }
    void addElement(XMLElement* el) { m_el_v.push_back(el); }
    ElementList& getElements() { return m_el_v; }

  private:
    std::string m_tag;
    AttributeList m_attr_m;
    ElementList m_el_v;

  };

}

#endif
