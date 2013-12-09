#include "daq/slc/readout/SharedMemory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

using namespace Belle2;

bool SharedMemory::unlink(const std::string& path) throw(IOException)
{
  return (::shm_unlink(path.c_str()) == -1);
}

SharedMemory::SharedMemory() throw()
  : FileDescriptor(), _path(), _size(0), _addr(NULL) {}

SharedMemory::SharedMemory(const std::string& path, size_t size)
throw(IOException) : FileDescriptor(),
  _path(path), _size(size), _addr(NULL)
{
}

SharedMemory::SharedMemory(const SharedMemory& file)
throw() : FileDescriptor(file._fd), _path(file._path),
  _size(file._size), _addr(file._addr) {}

SharedMemory::~SharedMemory() throw() {}

void SharedMemory::open(const std::string& path, size_t size) throw(IOException)
{
  errno = 0;
  int fd = ::shm_open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
  if (fd < 0) {
    if (errno != EEXIST) {
      throw (IOException(__FILE__, __LINE__, "Failed to open shared memory"));
    }
    fd = ::shm_open(path.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
      throw (IOException(__FILE__, __LINE__, "Failed to open shared memory"));
    }
  }
  ::ftruncate(fd, size);
  _fd = fd;
  _path = path;
  _size = size;
}

void SharedMemory::open() throw(IOException)
{
  open(_path, _size);
}

void* SharedMemory::map(size_t offset, size_t size) throw(IOException)
{
  void* addr = ::mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_SHARED, _fd, offset);
  if (addr == MAP_FAILED) {
    throw (IOException(__FILE__, __LINE__, "Failed to map sharemeory"));
  }
  _addr = addr;
  return addr;
}

void* SharedMemory::map() throw(IOException)
{
  if (_addr == NULL) _addr = map(0, _size);
  return _addr;
}

bool SharedMemory::unlink() throw()
{
  close();
  return (unlink(_path));
}

void SharedMemory::seekTo(size_t offset) throw(IOException)
{
  if (lseek(_fd, offset, SEEK_SET) == -1) {
    throw (IOException(__FILE__, __LINE__, "Failed to seek offset of the file"));
  }
}
void SharedMemory::seekBy(size_t offset) throw(IOException)
{
  if (lseek(_fd, offset, SEEK_CUR) == -1) {
    throw (IOException(__FILE__, __LINE__, "Failed to seek offset of the file "));
  }
}

bool SharedMemory::isOpened() throw()
{
  return (_fd != 0);
}

size_t SharedMemory::write(const void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;

  if (_fd == 0) {
    throw (IOException(__FILE__, __LINE__, "SharedMemory is not ready for writing."));
  }
  while (c < count) {
    ret = ::write(_fd, ((unsigned char*)buf + c), (count - c));
    if (ret  <= 0) {
      throw (IOException(__FILE__, __LINE__, "Error during data writing."));
    } else {
      c += ret;
    }
  }
  return c;
}

size_t SharedMemory::read(void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;

  if (_fd == 0) {
    throw (IOException(__FILE__, __LINE__, "SharedMemory is not ready for reading."));
  }
  while (c < count) {
    ret = ::read(_fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      throw (IOException(__FILE__, __LINE__, "Error during data reading."));
    } else {
      c += ret;
    }
  }
  return c;
}

const SharedMemory& SharedMemory::operator=(const SharedMemory& file) throw()
{
  _fd = file._fd;
  _path = file._path;
  _size = file._size;
  _addr = file._addr;
  return *this;
}

