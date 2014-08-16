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
    SharedMemory(const std::string& path, size_t size);
    SharedMemory(const SharedMemory&) throw();
    ~SharedMemory() throw();

    // member functions;
  public:
    bool open(const std::string& path, size_t size);
    bool open();
    void close();
    bool unlink();
    bool seekTo(size_t);
    bool seekBy(size_t);
    void* map(size_t, size_t);
    void* map();
    bool isOpened() throw();
    const std::string& getPath() const throw() { return m_path; }
    size_t size() const throw() { return m_size; }
    int getfd() const throw() { return m_fd; }

    // operators
  public:
    const SharedMemory& operator=(const SharedMemory&) throw();

    // data members;
  private:
    int m_fd;
    std::string m_path;
    size_t m_size;
    void* m_addr;

  };

};

#endif
