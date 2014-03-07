#ifndef _Belle2_BufferedWriter_hh
#define _Belle2_BufferedWriter_hh

#include "daq/slc/base/Writer.h"

namespace Belle2 {

  class BufferedWriter : public Writer {

    // member data
  protected:
    unsigned char* _memory;
    size_t _size;
    size_t _pos;
    bool _allocated;

    // constructors & destructors
  public:
    BufferedWriter() throw();
    BufferedWriter(size_t size, unsigned char* memory = NULL) throw();
    BufferedWriter(const BufferedWriter& w) throw();
    virtual ~BufferedWriter() throw();

    // member methods implemented
  public:
    void seekTo(size_t pos) throw() { _pos = pos; }
    unsigned char* ptr() throw() { return _memory; }
    size_t count() const throw() { return _pos; }
    size_t size() const throw() { return _size; }
    virtual size_t write(const void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }

    // operators
  public:
    const BufferedWriter& operator = (const BufferedWriter&) throw();

  };

}

#endif
