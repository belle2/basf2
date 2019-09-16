#ifndef _Belle2_StreamSizeCounter_hh
#define _Belle2_StreamSizeCounter_hh

#include "daq/slc/base//Writer.h"

namespace Belle2 {

  class StreamSizeCounter : public Writer {

  public:
    StreamSizeCounter() : m_count(0) {}
    virtual ~StreamSizeCounter() {}

  public:
    virtual size_t write(const void*, size_t);
    virtual bool available() { return true; }
    void reset() { m_count = 0; }
    int count() const { return m_count; }

  private:
    size_t m_count;

  };

}

#endif

