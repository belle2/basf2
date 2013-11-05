#ifndef _Belle2_FileReader_hh
#define _Belle2_FileReader_hh

#include "system/FileDescriptor.h"

#include "base/Reader.h"

namespace Belle2 {

  template <class FD>
  class FileReader : public Reader {

  public:
    FileReader() throw() {}
    FileReader(const FD& fd) throw() : _fd(fd) {}
    virtual ~FileReader() throw() {}

  public:
    virtual size_t read(void* v, size_t count)
    throw(IOException) {
      return _fd.read(v, count);
    }
    virtual bool available() throw(IOException) {
      return _fd.select();
    }

  private:
    FD _fd;

  };

};

#endif
