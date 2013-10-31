#ifndef _Belle2_FileDescriptor_hh
#define _Belle2_FileDescriptor_hh

#include "base/IOException.h"

namespace Belle2 {

  class FileDescriptor {

  protected:
    int _fd;

  public:
    FileDescriptor() throw();
    FileDescriptor(const FileDescriptor&) throw();
    FileDescriptor(int fd) throw();
    virtual ~FileDescriptor() throw();

  public:
    int get_fd() const throw();
    bool select(int sec = -1, int usec = -1) throw(IOException);
    bool close() throw();

  };

}

#endif
