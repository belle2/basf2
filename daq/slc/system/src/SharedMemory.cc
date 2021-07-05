/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/SharedMemory.h"

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

SharedMemory::SharedMemory()
  : m_fd(-1), m_path(), m_size(0), m_addr(NULL) {}

SharedMemory::SharedMemory(const std::string& path, size_t size)
  : m_fd(-1), m_path(path), m_size(size), m_addr(NULL)
{
}

SharedMemory::SharedMemory(const SharedMemory& file)
  : m_fd(file.m_fd),  m_path(file.m_path),
    m_size(file.m_size), m_addr(file.m_addr) {}

SharedMemory::~SharedMemory() {}

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
  m_fd = fd;
  m_path = path;
  truncate(size);
  return true;
}

bool SharedMemory::open()
{
  return open(m_path, m_size);
}

void SharedMemory::close()
{
  if (m_fd > 0) {
    if (m_addr != NULL) munmap(m_addr, m_size);
    ::close(m_fd);
    m_fd = 0;
  }
}

bool SharedMemory::truncate(size_t size)
{
  if (size > 0) {
    ::ftruncate(m_fd, size);
    m_size = size;
    return true;
  } else {
    struct stat st;
    fstat(m_fd, &st);
    m_size = st.st_size;
  }
  return false;
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
  m_size = size;
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

bool SharedMemory::isOpened()
{
  return (m_fd != 0);
}

const SharedMemory& SharedMemory::operator=(const SharedMemory& file)
{
  m_fd = file.m_fd;
  m_path = file.m_path;
  m_size = file.m_size;
  m_addr = file.m_addr;
  return *this;
}

