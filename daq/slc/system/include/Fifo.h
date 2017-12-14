#ifndef _Belle2_Fifo_hh
#define _Belle2_Fifo_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class Fifo : public FileDescriptor {

  public:
    static Fifo mkfifo(const std::string& path) throw(IOException);

  public:
    Fifo() throw() {}
    virtual ~Fifo() throw() {}

  public:
    void open(const std::string& path,
              const std::string& mode = "r") throw(IOException);
    void unlink(const std::string& path) throw(IOException);
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  };

}

#endif
