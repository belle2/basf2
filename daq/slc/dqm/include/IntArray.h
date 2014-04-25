#ifndef _Belle2_IntArray_h
#define _Belle2_IntArray_h

#include "daq/slc/dqm/NumberArray.h"

namespace Belle2 {

  class IntArray : public NumberArray {

    // constructors & destructors
  public:
    IntArray() throw();
    IntArray(const size_t size) throw();
    IntArray(const NumberArray& v) throw();
    virtual ~IntArray() throw();

  public:
    virtual double get(size_t n) const throw();
    virtual void* ptr() throw() { return m_value_v; }
    virtual const void* ptr() const throw() { return m_value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    int& operator[](size_t n) throw();
    int operator[](size_t n) const throw();

  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);
    virtual double readElement(Reader& reader) throw(IOException);
    virtual void writeElement(double value, Writer& writer) const throw(IOException);

    // member data
  private:
    int* m_value_v;

  };

};

#endif
