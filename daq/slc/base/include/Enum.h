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
    throw() : _id(id), _label(label) {}
    Enum(const Enum& e) throw() :
      _id(e._id), _label(e._label) {}
    Enum() throw() : _id(UNKNOWN._id),
      _label(UNKNOWN._label) {}

  public:
    int getId() const throw() { return _id; }
    const char* getLabel() const throw() { return _label.c_str(); }
    const std::string& getLabelString() const throw() { return _label; }

  public:
    bool operator==(const Enum& e) const throw();
    bool operator!=(const Enum& e) const throw();
    const Enum& operator=(const Enum& e) throw();

  protected:
    int _id;
    std::string _label;

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
    return *this;
  }

}

#endif
