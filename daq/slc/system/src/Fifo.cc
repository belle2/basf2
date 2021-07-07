/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Fifo.h"

#include <daq/slc/base/IOException.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdio>

using namespace Belle2;

Fifo Fifo::mkfifo(const std::string& path)
{
  ::mkfifo(path.c_str(), 0666);
  Fifo fifo;
  fifo.open(path.c_str());
  return fifo;
}

void Fifo::open(const std::string& path, const std::string& mode_s)
{
  int mode = O_RDONLY;
  if (mode_s.find("w") != std::string::npos) {
    mode = O_WRONLY;
    if (mode_s.find("r") != std::string::npos) {
      mode = O_RDWR;
    }
  }
  if ((m_fd = ::open(path.c_str(), mode)) < 0) {
    perror("open");
    throw (IOException("Failed to open fifo."));
  }
}

void Fifo::unlink(const std::string& path)
{
  if ((::unlink(path.c_str())) < 0) {
    perror("unlink");
  }
  close();
}

size_t Fifo::write(const void* buf, size_t count)
{
  return ::write(m_fd, buf, count);
}

size_t Fifo::read(void* buf, size_t count)
{
  return ::read(m_fd, buf, count);
}
