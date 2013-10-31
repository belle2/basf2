#ifndef _Belle2_Enum_hh
#define _Belle2_Enum_hh

#include <string>

namespace Belle2 {

  class State;

  class Enum {

  public:
    static const Enum UNKNOWN;

  public:
    ~Enum() throw() {}

  protected:
    Enum(int id, const std::string& label, const std::string& alias = "")
    throw() : _id(id), _label(label), _alias(alias) {}
    Enum(const Enum& e)
    throw() : _id(e._id), _label(e._label), _alias(e._alias) {}
    Enum()
    throw() : _id(UNKNOWN._id), _label(UNKNOWN._label), _alias(UNKNOWN._alias) {}

  public:
    int getId() const throw() { return _id; }
    const char* getLabel() const throw() { return _label.c_str(); }
    const char* getAlias() const throw() { return _alias.c_str(); }
    const std::string& getLabelString() const throw() { return _label; }
    const std::string& getAliasString() const throw() { return _alias; }

  public:
    bool operator==(const Enum& e) const throw();
    bool operator!=(const Enum& e) const throw();
    const Enum& operator=(const Enum& e) throw();

  protected:
    int _id;
    std::string _label, _alias;

  };

  inline bool Enum::operator==(const Enum& e) const throw()
  {
    return (_id == e._id);
  }
  inline bool Enum::operator!=(const Enum& e) const throw()
  {
    return (_id != e._id);
  }
  inline const Enum& Enum::operator=(const Enum& e) throw()
  {
    _id = e._id;
    _label = e._label;
    _alias = e._alias;
    return *this;
  }

}

#endif
