#include "daq/slc/system/FileDescriptor.h"

#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>

using namespace Belle2;

FileDescriptor::FileDescriptor() throw()
{
  m_fd = -1;
}

FileDescriptor::FileDescriptor(const FileDescriptor& fd) throw()
{
  m_fd = fd.m_fd;
}

FileDescriptor::FileDescriptor(int fd) throw()
{
  m_fd = fd;
}

FileDescriptor::~FileDescriptor() throw()
{

}

int FileDescriptor::get_fd() const throw()
{
  return m_fd;
}

bool FileDescriptor::select(int sec, int usec) throw(IOException)
{
  if (m_fd <= 0) {
    return false;
  }
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(m_fd, &fds);
  int ret;
  if (sec >= 0 && usec >= 0) {
    timeval t = {sec, usec};
    ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
  } else {
    ret = ::select(FD_SETSIZE, &fds, NULL, NULL, NULL);
  }
  if (ret < 0) {
    perror("select");
    throw (IOException("Failed to select"));
  }
  if (FD_ISSET(m_fd, &fds)) {
    return true;
  } else {
    return false;
  }
}

bool FileDescriptor::close() throw()
{
  if (m_fd > 0) {
    if (::close(m_fd) != 0) {
      return false;
    }
  }
  m_fd = -1;
  return true;
}
