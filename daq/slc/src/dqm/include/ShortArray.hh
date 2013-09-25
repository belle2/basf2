#ifndef ShortArray_hh
#define ShortArray_hh

#include "NumberArray.hh"

namespace B2DQM { 

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
    virtual void readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual double readElement(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeElement(double value, B2DAQ::Writer& writer)
      const throw(B2DAQ::IOException);

    // member data
  private:
    short* _value_v;
 
  };
 
};
 
#endif
