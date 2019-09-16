#ifndef _Belle2_BufferedReader_hh
#define _Belle2_BufferedReader_hh

#include "daq/slc/base/Reader.h"

namespace Belle2 {

  class BufferedReader : public Reader {

    // constructors & destructors
  public:
    BufferedReader();
    BufferedReader(size_t size, unsigned char* memory = NULL);
    BufferedReader(const BufferedReader& r);
    virtual ~BufferedReader();

    // member methods implemented
  public:
    void seekTo(size_t pos) { m_pos = pos; }
    unsigned char* ptr() { return m_memory; }
    size_t count() const { return m_pos; }
    size_t size() const { return m_size; }
    void copy(const void*, size_t);
    virtual size_t read(void*, size_t);
    virtual bool available() { return true; }

  public:
    const BufferedReader& operator = (const BufferedReader&);

    // member data
  protected:
    unsigned char* m_memory;
    size_t m_size;
    size_t m_pos;
    bool m_allocated;

  };

}

#endif
