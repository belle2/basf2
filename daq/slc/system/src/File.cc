/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/File.h"

#include <daq/slc/base/IOException.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <cstdio>

using namespace Belle2;

void File::open(const std::string& path, const std::string& mode_s)
{
  int mode = O_RDONLY;
  if (mode_s.find("w") != std::string::npos) {
    mode = O_WRONLY;
    if (mode_s.find("r") != std::string::npos) {
      mode = O_RDWR;
    } else {
      mode |= O_CREAT;
    }
  }
  if ((m_fd = ::open(path.c_str(), mode, 0666)) < 0) {
    perror("open");
    throw (IOException("Failed to open file : %s", path.c_str()));
  }
}

void File::unlink(const std::string& path)
{
  if ((::unlink(path.c_str())) < 0) {
    perror("unlink");
  }
  close();
}

size_t File::write(const void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = ::write(m_fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          throw (IOException("Error while writing"));
      }
    }
    c += ret;
  }
  return c;
}

size_t File::read(void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = ::read(m_fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          throw (IOException("Error while reading. %d", errno));
      }
    }
    c += ret;
  }
  return c;
}

bool File::exist(const std::string& filename)
{
  struct stat st;
  if (stat(filename.c_str(), &st) != 0) {
    return false;
  } else {
    mode_t m = st.st_mode;
    if (S_ISDIR(m)) {
      return false;
    } else {
      return true;
    }
  }
  return false;
}
