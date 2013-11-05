#ifndef _Belle2_FloatArray_hh
#define _Belle2_FloatArray_hh

#include "dqm/NumberArray.h"

namespace Belle2 {

  class FloatArray : public NumberArray {

    // constructors & destructors
  public:
    FloatArray() throw();
    FloatArray(const size_t size) throw();
    FloatArray(const NumberArray& v) throw();
    ~FloatArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return _value_v; }
    virtual const void* ptr() const throw() { return _value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    float& operator[](size_t n) throw();
    float operator[](size_t n) const throw();

  public:
    virtual void readObject(Belle2::Reader& reader)
    throw(Belle2::IOException);
    virtual double readElement(Belle2::Reader& reader)
    throw(Belle2::IOException);
    virtual void writeObject(Belle2::Writer& writer)
    const throw(Belle2::IOException);
    virtual void writeElement(double value, Belle2::Writer& writer)
    const throw(Belle2::IOException);

    // member data
  private:
    float* _value_v;

  };

};

#endif
