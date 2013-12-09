#ifndef _Belle2_BufferedReader_hh
#define _Belle2_BufferedReader_hh

#include "daq/slc/base/Reader.h"

namespace Belle2 {

  class BufferedReader : public Reader {

    // member data
  protected:
    unsigned char* _memory;
    size_t _size;
    size_t _pos;

    // constructors & destructors
  public:
    BufferedReader() throw();
    BufferedReader(size_t size) throw();
    BufferedReader(const BufferedReader& r) throw();
    virtual ~BufferedReader() throw();

    // member methods implemented
  public:
    void seekTo(size_t pos) throw() { _pos = pos; }
    unsigned char* ptr() throw() { return _memory; }
    size_t count() const throw() { return _pos; }
    size_t size() const throw() { return _size; }
    void copy(const void*, size_t) throw(IOException);
    virtual size_t read(void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }

  public:
    const BufferedReader& operator = (const BufferedReader&) throw();

  };

}

#endif
