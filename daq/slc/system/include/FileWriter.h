#ifndef _Belle2_FileWriter_hh
#define _Belle2_FileWriter_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/Writer.h"

namespace Belle2 {

  template <class FD>
  class FileWriter : public Writer {

  public:
    FileWriter() {}
    FileWriter(const FD& fd) : _fd(fd) {}
    virtual ~FileWriter() {}

  public:
    virtual size_t write(const void* v, size_t count)
    {
      return _fd.write(v, count);
    }
    virtual bool available()
    {
      return _fd.select();
    }

  private:
    FD _fd;

  };

};

#endif
