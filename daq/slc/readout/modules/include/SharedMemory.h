#ifndef _Belle2_SharedMemory_h
#define _Belle2_SharedMemory_h

#include <string>

namespace Belle2 {

  class SharedMemory {

  public:
    static bool unlink(const std::string& path);

    // constructors and destructor
  public:
    SharedMemory() throw();
    SharedMemory(const std::string&, size_t);
    SharedMemory(const SharedMemory&) throw();
    ~SharedMemory() throw();

    // member functions;
  public:
    bool open(const std::string&, size_t);
    bool open();
    void close();
    bool unlink();
    bool seekTo(size_t);
    bool seekBy(size_t);
    void* map(size_t, size_t);
    void* map();
    bool isOpened() throw();
    const std::string& getPath() const throw() { return _path; }
    size_t size() const throw() { return _size; }
    int getfd() const throw() { return _fd; }

    // operators
  public:
    const SharedMemory& operator=(const SharedMemory&) throw();

    // data members;
  private:
    int _fd;
    std::string _path;
    size_t _size;
    void* _addr;

  };

};

#endif
