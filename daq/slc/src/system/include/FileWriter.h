#ifndef _Belle2_FileWriter_hh
#define _Belle2_FileWriter_hh

#include "FileDescriptor.h"

#include "base/Writer.h"

namespace Belle2 {

  template <class FD>
  class FileWriter : public Writer {

  public:
    FileWriter() throw() {}
    FileWriter(const FD& fd) throw() : _fd(fd) {}
    virtual ~FileWriter() throw() {}

  public:
    virtual size_t write(const void* v, size_t count)
    throw(IOException) {
      return _fd.write(v, count);
    }
    virtual bool available() throw(IOException) {
      return _fd.select();
    }

  private:
    FD _fd;

  };

};

#endif
