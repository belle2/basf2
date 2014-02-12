//+
// File : SeqFile.cc
// Description : Sequential file I/O privitives with file blocking
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jun - 2008
//-

#include <framework/pcore/SeqFile.h>
#include <framework/logging/Logger.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

using namespace Belle2;
using namespace std;

SeqFile::SeqFile(const char* filename, const char* rwflag)
{
  if (strstr(rwflag, "w") != 0)
    m_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0664);
  else
    m_fd = open(filename, O_RDONLY);

  if (m_fd < 0) {
    B2ERROR("file open error (" << strerror(errno) << "): " << filename);
  }
  strcpy(m_filename, filename);
  B2INFO("SeqFile: " << m_filename << " opened (fd=" << m_fd << ")");
  m_nfile = 0;
  m_nb = 0;
}

SeqFile::~SeqFile()
{
  close(m_fd);
  B2INFO("Seq File " << m_nfile << " closed");
}

int SeqFile::status()
{
  return m_fd;
}

int SeqFile::write(char* buf)
{
  int stat = 0;
  int insize = *((int*)buf); // nbytes in the buffer at the beginning
  if (insize + m_nb >= BLOCKSIZE &&
      strcmp(m_filename, "/dev/null") != 0) {
    close(m_fd);
    B2INFO("SeqFile: previous file closed (size=" << m_nb << " bytes)");
    m_nfile++;
    char filename[256];
    sprintf(filename, "%s-%d", m_filename, m_nfile);
    m_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (m_fd < 0) {
      B2FATAL("file open error (" << strerror(errno) << "): " << filename);
    }
    m_nb = 0;
    B2INFO("SeqFile: " << m_filename << " opened");
    stat = ::write(m_fd, buf, insize);
    if (stat > 0)
      m_nb += stat;
  } else {
    stat = ::write(m_fd, buf, insize);
    if (stat > 0)
      m_nb += stat;
  }
  return stat;
}

int SeqFile::read(char* buf, int size)
{
  int recsize = 0;
  int stat = ::read(m_fd, buf, sizeof(int));    // record size
  //  printf ( "stat = %d, recsize = %d\n", stat, recsize );
  if (stat < 0) {
    // Error in reading file
    perror("read");
    return stat;
  } else if (stat == 0) {
    // EOF of current file, search for next file
    close(m_fd);
    m_nfile++;
    char nextfile[256];
    sprintf(nextfile, "%s-%d", m_filename, m_nfile);
    m_fd = open(nextfile, O_RDONLY);
    if (m_fd < 0) return 0;   // End of all files
    // Obtain record size from new file
    int stat2 = ::read(m_fd, buf, sizeof(int));
    if (stat2 < 0) {
      perror("read2");
      return stat2;
    }
    recsize = *((int*)buf);
    if (recsize > size) {
      fprintf(stderr, "read3: buffer too small\n");
      return -1;
    }
    // Obtain body
    int stat3 = ::read(m_fd, buf + sizeof(int), recsize - sizeof(int));
    return stat3 + sizeof(int);
  } else {
    // Normal processing
    recsize = *((int*)buf);
    if (recsize > size) {
      fprintf(stderr, "read4: buffer too small\n");
      return -1;
    }
    stat = ::read(m_fd, buf + sizeof(int), recsize - sizeof(int));
    if (stat < 0) perror("read4");
    return stat;
  }
}
