#ifndef DoubleArray_hh
#define DoubleArray_hh

#include "NumberArray.h"

namespace Belle2 {

  class DoubleArray : public NumberArray {

    // constructors & destructors
  public:
    DoubleArray() throw();
    DoubleArray(const size_t size) throw();
    DoubleArray(const NumberArray& v) throw();
    virtual ~DoubleArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return _value_v; }
    virtual const void* ptr() const throw() { return _value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    double& operator[](size_t n) throw();
    double operator[](size_t n) const throw();

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
    double* _value_v;

  };

};

#endif
