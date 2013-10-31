#ifndef _Belle2_NumberArray_hh
#define _Belle2_NumberArray_hh

#include "base/Reader.h"
#include "base/Writer.h"

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
    size_t size() const throw() { return _size; }
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
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException) = 0;
    virtual void writeElement(double value, Belle2::Writer& writer) const throw(Belle2::IOException) = 0;
    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException) = 0;
    virtual double readElement(Belle2::Reader& reader) throw(Belle2::IOException) = 0;

    // member data
  protected:
    size_t _size;

  };

};

#endif
