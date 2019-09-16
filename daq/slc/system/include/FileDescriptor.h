#ifndef _Belle2_FileDescriptor_hh
#define _Belle2_FileDescriptor_hh

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class FileDescriptor {

  public:
    FileDescriptor();
    FileDescriptor(const FileDescriptor&);
    FileDescriptor(int fd);
    virtual ~FileDescriptor();

  public:
    int get_fd() const;
    bool select(int sec = -1, int usec = -1);
    bool select2(int sec = -1, int usec = -1);
    bool close();

  protected:
    int m_fd;

  };

}

#endif
