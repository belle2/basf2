#ifndef _B2DAQ_Fifo_hh
#define _B2DAQ_Fifo_hh

#include "FileDescriptor.hh"

#include <util/IOException.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace B2DAQ {

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
