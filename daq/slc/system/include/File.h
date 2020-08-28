#ifndef _Belle2_File_hh
#define _Belle2_File_hh

#include "daq/slc/system/FileDescriptor.h"

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class File : public FileDescriptor {

  public:
    static bool exist(const std::string& filename);

  public:
    File() {}
    File(int fd) : FileDescriptor(fd) {}
    File(const std::string& path,
         const std::string& mode = "r") { open(path, mode); }
    virtual ~File() {}

  public:
    void open(const std::string& path,
              const std::string& mode = "r");
    void unlink(const std::string& path);
    virtual size_t write(const void* v, size_t count);
    virtual size_t read(void* v, size_t count);

  };

}

#endif
