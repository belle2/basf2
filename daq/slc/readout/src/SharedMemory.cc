#include "daq/slc/readout/SharedMemory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

using namespace Belle2;

bool SharedMemory::unlink(const std::string& path)
{
  return (::shm_unlink(path.c_str()) == -1);
}

SharedMemory::SharedMemory() throw()
  : _fd(-1), _path(), _size(0), _addr(NULL) {}

SharedMemory::SharedMemory(const std::string& path, size_t size)
  : _fd(-1), _path(path), _size(size), _addr(NULL)
{
}

SharedMemory::SharedMemory(const SharedMemory& file)
throw() : _fd(file._fd),  _path(file._path),
  _size(file._size), _addr(file._addr) {}

SharedMemory::~SharedMemory() throw() {}

bool SharedMemory::open(const std::string& path, size_t size)
{
  errno = 0;
  int fd = ::shm_open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
  if (fd < 0) {
    if (errno != EEXIST) {
      return false;
    }
    fd = ::shm_open(path.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
      return false;
    }
  }
  ::ftruncate(fd, size);
  _fd = fd;
  _path = path;
  _size = size;
}

bool SharedMemory::open()
{
  return open(_path, _size);
}

void SharedMemory::close()
{
  if (_fd > 0) ::close(_fd);
}

void* SharedMemory::map(size_t offset, size_t size)
{
  void* addr = ::mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_SHARED, _fd, offset);
  if (addr == MAP_FAILED) {
    addr = NULL;
  }
  _addr = addr;
  return addr;
}

void* SharedMemory::map()
{
  if (_addr == NULL) _addr = map(0, _size);
  return _addr;
}

bool SharedMemory::unlink()
{
  close();
  return (unlink(_path));
}

bool SharedMemory::seekTo(size_t offset)
{
  return (lseek(_fd, offset, SEEK_SET) == -1);
}

bool SharedMemory::seekBy(size_t offset)
{
  return (lseek(_fd, offset, SEEK_CUR) == -1);
}

bool SharedMemory::isOpened() throw()
{
  return (_fd != 0);
}

const SharedMemory& SharedMemory::operator=(const SharedMemory& file) throw()
{
  _fd = file._fd;
  _path = file._path;
  _size = file._size;
  _addr = file._addr;
  return *this;
}

