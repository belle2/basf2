#include "daq/slc/system/SharedMemory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

using namespace Belle2;

bool SharedMemory::unlink(const std::string& path)
{
  return (::shm_unlink(path.c_str()) == -1);
}

SharedMemory::SharedMemory() throw()
  : m_fd(-1), m_path(), m_size(0), m_addr(NULL) {}

SharedMemory::SharedMemory(const std::string& path, size_t size)
  : m_fd(-1), m_path(path), m_size(size), m_addr(NULL)
{
}

SharedMemory::SharedMemory(const SharedMemory& file)
throw() : m_fd(file.m_fd),  m_path(file.m_path),
  m_size(file.m_size), m_addr(file.m_addr) {}

SharedMemory::~SharedMemory() throw() {}

bool SharedMemory::open(const std::string& path, size_t size)
{
  errno = 0;
  int fd = ::shm_open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
  if (fd < 0) {
    if (errno != EEXIST) {
      perror("shm_oepn");
      return false;
    }
    fd = ::shm_open(path.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
      perror("shm_oepn");
      return false;
    }
  }
  struct stat st;
  fstat(fd, &st);
  if (st.st_size < (int)size) {
    ::ftruncate(fd, size);
  } else {
    size = st.st_size;
  }
  m_fd = fd;
  m_path = path;
  m_size = size;
  return true;
}

bool SharedMemory::open()
{
  return open(m_path, m_size);
}

void SharedMemory::close()
{
  if (m_fd > 0) ::close(m_fd);
}

void* SharedMemory::map(size_t offset, size_t size)
{
  errno = 0;
  void* addr = ::mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_SHARED, m_fd, offset);
  if (addr == MAP_FAILED) {
    perror("mmap");
    addr = NULL;
  }
  m_addr = addr;
  return addr;
}

void* SharedMemory::map()
{
  if (m_addr == NULL) m_addr = map(0, m_size);
  return m_addr;
}

bool SharedMemory::unlink()
{
  close();
  return (unlink(m_path));
}

bool SharedMemory::seekTo(size_t offset)
{
  return (lseek(m_fd, offset, SEEK_SET) == -1);
}

bool SharedMemory::seekBy(size_t offset)
{
  return (lseek(m_fd, offset, SEEK_CUR) == -1);
}

bool SharedMemory::isOpened() throw()
{
  return (m_fd != 0);
}

const SharedMemory& SharedMemory::operator=(const SharedMemory& file) throw()
{
  m_fd = file.m_fd;
  m_path = file.m_path;
  m_size = file.m_size;
  m_addr = file.m_addr;
  return *this;
}

