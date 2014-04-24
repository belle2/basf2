#ifndef _Belle2_StreamSizeCounter_hh
#define _Belle2_StreamSizeCounter_hh

#include "daq/slc/base//Writer.h"

namespace Belle2 {

  class StreamSizeCounter : public Writer {

  public:
    StreamSizeCounter() : _count(0) {}
    virtual ~StreamSizeCounter() throw() {}

  public:
    virtual size_t write(const void*, size_t) throw(IOException);
    virtual bool available() throw() { return true; }
    void reset() throw() { _count = 0; }
    int count() const throw() { return _count; }

  private:
    size_t _count;

  };

}

#endif

