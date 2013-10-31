#ifndef _Belle2_Fifo_hh
#define _Belle2_Fifo_hh

#include "FileDescriptor.h"

#include "base/IOException.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace Belle2 {

  class Fifo : public FileDescriptor {

  public:
    static Fifo mkfifo(const char* path, int opt = O_RDWR) throw(IOException);

  public:
    Fifo() throw() {}
    virtual ~Fifo() throw() {}

  public:
    void open(const char* path, int opt = O_RDWR) throw(IOException);
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  };

}

#endif
