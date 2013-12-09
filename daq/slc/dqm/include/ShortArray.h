#ifndef ShortArray_hh
#define ShortArray_hh

#include "daq/slc/dqm/NumberArray.h"

namespace Belle2 {

  class ShortArray : public NumberArray {

    // constructors & destructors
  public:
    ShortArray() throw();
    ShortArray(const size_t size) throw();
    ShortArray(const NumberArray& v) throw();
    virtual ~ShortArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return _value_v; }
    virtual const void* ptr() const throw() { return _value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    short& operator[](size_t n) throw();
    short operator[](size_t n) const throw();

  public:
    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual double readElement(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeElement(double value, Belle2::Writer& writer)
    const throw(Belle2::IOException);

    // member data
  private:
    short* _value_v;

  };

};

#endif
