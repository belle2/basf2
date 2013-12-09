#ifndef _Belle2_SharedMemory_h
#define _Belle2_SharedMemory_h

#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class SharedMemory : public FileDescriptor {

  public:
    static bool unlink(const std::string& path) throw(IOException);

    // data members;
  private:
    std::string _path;
    size_t _size;
    void* _addr;

    // constructors and destructor
  public:
    SharedMemory() throw();
    SharedMemory(const std::string&, size_t) throw(IOException);
    SharedMemory(const SharedMemory&) throw();
    ~SharedMemory() throw();

    // member functions;
  public:
    void open(const std::string&, size_t) throw(IOException);
    void open() throw(IOException);
    bool unlink() throw();
    void seekTo(size_t) throw(IOException);
    void seekBy(size_t) throw(IOException);
    void* map(size_t, size_t) throw(IOException);
    void* map() throw(IOException);
    bool isOpened() throw();
    const std::string& getPath() const throw() { return _path; }
    size_t size() const throw() { return _size; }

    // meber function implemented from AbstractSharedMemory
  public:
    size_t write(const void*, size_t) throw(IOException);
    size_t read(void*, size_t) throw(IOException);
    int getfd() const throw() { return _fd; }

    // operators
  public:
    const SharedMemory& operator=(const SharedMemory&) throw();

  };

};

#endif
