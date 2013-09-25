#ifndef _B2DAQ_StreamSizeCounter_hh
#define _B2DAQ_StreamSizeCounter_hh

#include <util/Writer.hh>

namespace B2DAQ {

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

