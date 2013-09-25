#ifndef DoubleArray_hh
#define DoubleArray_hh

#include "NumberArray.hh"

namespace B2DQM { 

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
    virtual void readObject(B2DAQ::Reader& reader)
      throw(B2DAQ::IOException);
    virtual double readElement(B2DAQ::Reader& reader)
      throw(B2DAQ::IOException);
    virtual void writeObject(B2DAQ::Writer& writer)
      const throw(B2DAQ::IOException);
    virtual void writeElement(double value, B2DAQ::Writer& writer)
      const throw(B2DAQ::IOException);

    // member data
  private:
    double* _value_v;
 
  };
 
};
 
#endif
