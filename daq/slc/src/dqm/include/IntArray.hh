#ifndef _B2DQM_IntArray_hh
#define _B2DQM_IntArray_hh

#include "NumberArray.hh"

namespace B2DQM { 

  class IntArray : public NumberArray {

    // constructors & destructors 
  public:
    IntArray() throw();
    IntArray(const size_t size) throw();
    IntArray(const NumberArray& v) throw();
    virtual ~IntArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return _value_v; }
    virtual const void* ptr() const throw() { return _value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    int& operator[](size_t n) throw();
    int operator[](size_t n) const throw();

  public:
    virtual void readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual double readElement(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeElement(double value, B2DAQ::Writer& writer) const throw(B2DAQ::IOException);

    // member data
  private:
    int* _value_v;
 
  };
 
};
 
#endif
