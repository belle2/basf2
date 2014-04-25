#ifndef _Belle2_DoubleArray_h
#define _Belle2_DoubleArray_h

#include "daq/slc/dqm/NumberArray.h"

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
    virtual void* ptr() throw() { return m_value_v; }
    virtual const void* ptr() const throw() { return m_value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    double& operator[](size_t n) throw();
    double operator[](size_t n) const throw();

  public:
    virtual void readObject(Reader& reader)
    throw(IOException);
    virtual double readElement(Reader& reader)
    throw(IOException);
    virtual void writeObject(Writer& writer)
    const throw(IOException);
    virtual void writeElement(double value, Writer& writer)
    const throw(IOException);

    // member data
  private:
    double* m_value_v;

  };

};

#endif
