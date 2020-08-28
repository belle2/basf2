#ifndef _Belle2_Fifo_hh
#define _Belle2_Fifo_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class Fifo : public FileDescriptor {

  public:
    static Fifo mkfifo(const std::string& path);

  public:
    Fifo() {}
    virtual ~Fifo() {}

  public:
    void open(const std::string& path,
              const std::string& mode = "r");
    void unlink(const std::string& path);
    virtual size_t write(const void* v, size_t count);
    virtual size_t read(void* v, size_t count);

  };

}

#endif
