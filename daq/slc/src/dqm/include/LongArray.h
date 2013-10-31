#ifndef _Belle2_LongArray_hh
#define _Belle2_LongArray_hh

#include "NumberArray.h"

namespace Belle2 {

  class LongArray : public NumberArray {

    // constructors & destructors
  public:
    LongArray() throw();
    LongArray(const size_t size) throw();
    LongArray(const NumberArray& v) throw();
    virtual ~LongArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return _value_v; }
    virtual const void* ptr() const throw() { return _value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    long long& operator[](size_t n) throw();
    long long operator[](size_t n) const throw();

  public:
    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual double readElement(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeElement(double value, Belle2::Writer& writer) const throw(Belle2::IOException);

    // member data
  private:
    long long* _value_v;

  };

};

#endif
