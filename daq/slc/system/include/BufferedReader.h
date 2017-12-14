#ifndef _Belle2_BufferedReader_hh
#define _Belle2_BufferedReader_hh

#include "daq/slc/base/Reader.h"

namespace Belle2 {

  class BufferedReader : public Reader {

    // constructors & destructors
  public:
    BufferedReader() throw();
    BufferedReader(size_t size, unsigned char* memory = NULL) throw();
    BufferedReader(const BufferedReader& r) throw();
    virtual ~BufferedReader() throw();

    // member methods implemented
  public:
    void seekTo(size_t pos) throw() { m_pos = pos; }
    unsigned char* ptr() throw() { return m_memory; }
    size_t count() const throw() { return m_pos; }
    size_t size() const throw() { return m_size; }
    void copy(const void*, size_t) throw(IOException);
    virtual size_t read(void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }

  public:
    const BufferedReader& operator = (const BufferedReader&) throw();

    // member data
  protected:
    unsigned char* m_memory;
    size_t m_size;
    size_t m_pos;
    bool m_allocated;

  };

}

#endif
