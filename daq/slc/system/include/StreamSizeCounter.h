#ifndef _Belle2_StreamSizeCounter_hh
#define _Belle2_StreamSizeCounter_hh

#include "daq/slc/base//Writer.h"

namespace Belle2 {

  class StreamSizeCounter : public Writer {

  public:
    StreamSizeCounter() : m_count(0) {}
    virtual ~StreamSizeCounter() throw() {}

  public:
    virtual size_t write(const void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }
    void reset() throw() { m_count = 0; }
    int count() const throw() { return m_count; }

  private:
    size_t m_count;

  };

}

#endif

