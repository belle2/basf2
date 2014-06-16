#ifndef _Belle2_BufferedWriter_hh
#define _Belle2_BufferedWriter_hh

#include "daq/slc/base/Writer.h"

namespace Belle2 {

  class BufferedWriter : public Writer {

    // constructors & destructors
  public:
    BufferedWriter() throw();
    BufferedWriter(size_t size, unsigned char* memory = NULL) throw();
    BufferedWriter(const BufferedWriter& w) throw();
    virtual ~BufferedWriter() throw();

    // member methods implemented
  public:
    void seekTo(size_t pos) throw() { m_pos = pos; }
    unsigned char* ptr() throw() { return m_memory; }
    size_t count() const throw() { return m_pos; }
    size_t size() const throw() { return m_size; }
    virtual size_t write(const void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }

    // operators
  public:
    const BufferedWriter& operator = (const BufferedWriter&) throw();

    // member data
  protected:
    unsigned char* m_memory;
    size_t m_size;
    size_t m_pos;
    bool m_allocated;

  };

}

#endif
