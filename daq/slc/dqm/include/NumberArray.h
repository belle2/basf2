#ifndef _Belle2_NumberArray_h
#define _Belle2_NumberArray_h

#include "daq/slc/base/Reader.h"
#include "daq/slc/base/Writer.h"

namespace Belle2 {

  class NumberArray {

    // constructors & destructors
  public:
    NumberArray() throw();
    NumberArray(size_t len) throw();
    NumberArray(const NumberArray& v) throw();
    virtual ~NumberArray() throw();

    // constructors & destructors
  public:
    size_t size() const throw() { return m_size; }
    virtual double get(size_t n) const throw() = 0;
    virtual void* ptr() throw() = 0;
    virtual const void* ptr() const throw() = 0;
    virtual void resize(int size) = 0;
    virtual void set(size_t n, const double v) throw() = 0;
    void setAll(double v) throw();
    void copy(const NumberArray& v) throw();
    virtual std::string toString() const throw();

  public:
    const NumberArray& operator=(const NumberArray& v) throw();

  public:
    virtual void writeObject(Writer& writer) const throw(IOException) = 0;
    virtual void writeElement(double value, Writer& writer) const throw(IOException) = 0;
    virtual void readObject(Reader& reader) throw(IOException) = 0;
    virtual double readElement(Reader& reader) throw(IOException) = 0;

    // member data
  protected:
    size_t m_size;

  };

};

#endif
