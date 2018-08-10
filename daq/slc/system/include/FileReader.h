#ifndef _Belle2_FileReader_hh
#define _Belle2_FileReader_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/Reader.h"

namespace Belle2 {

  template <class FD>
  class FileReader : public Reader {

  public:
    FileReader() {}
    FileReader(const FD& fd) : _fd(fd) {}
    virtual ~FileReader() {}

  public:
    virtual size_t read(void* v, size_t count) { return _fd.read(v, count); }
    virtual bool available() { return _fd.select(); }

  private:
    FD _fd;

  };

};

#endif
