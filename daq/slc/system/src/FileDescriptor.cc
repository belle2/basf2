/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/FileDescriptor.h"

#include <daq/slc/base/IOException.h>

#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>

using namespace Belle2;

FileDescriptor::FileDescriptor()
{
  m_fd = -1;
}

FileDescriptor::FileDescriptor(const FileDescriptor& fd)
{
  m_fd = fd.m_fd;
}

FileDescriptor::FileDescriptor(int fd)
{
  m_fd = fd;
}

FileDescriptor::~FileDescriptor()
{

}

int FileDescriptor::get_fd() const
{
  return m_fd;
}

bool FileDescriptor::select(int sec, int usec)
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

bool FileDescriptor::select2(int sec, int usec)
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
    ret = ::select(FD_SETSIZE, NULL, &fds, NULL, &t);
  } else {
    ret = ::select(FD_SETSIZE, NULL, &fds, NULL, NULL);
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

bool FileDescriptor::close()
{
  if (m_fd > 0) {
    if (::close(m_fd) != 0) {
      return false;
    }
  }
  m_fd = -1;
  return true;
}
