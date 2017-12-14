#ifndef _Belle2_LongArray_h
#define _Belle2_LongArray_h

#include "daq/slc/dqm/NumberArray.h"

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
    virtual void* ptr() throw() { return m_value_v; }
    virtual const void* ptr() const throw() { return m_value_v; }
    virtual void set(size_t n, double v) throw();
    virtual void resize(int s) throw();
    virtual std::string toScript() const throw();

  public:
    long long& operator[](size_t n) throw();
    long long operator[](size_t n) const throw();

  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);
    virtual double readElement(Reader& reader) throw(IOException);
    virtual void writeElement(double value, Writer& writer) const throw(IOException);

    // member data
  private:
    long long* m_value_v;

  };

};

#endif
