//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <framework/logging/Logger.h>
#include <framework/pcore/SeqFile.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/PThread.h>

#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Belle2;

const unsigned long long MAX_FILE_SIZE = 2048000000;
int g_fd = 0;
int g_nfile = 0;

class FileCloser {

public:
  FileCloser(int fd, const std::string& dir, int expno, int runno)
    : m_fd(fd), m_expno(expno), m_runno(runno), m_dir(dir) {}

public:
  void run() {
    printf("closing old file\n");
    if (m_fd > 0) close(m_fd);
    char filename[1024];
    g_nfile++;
    sprintf(filename, "%s/e%4.4dr%6.6d.sroot-%d",
            m_dir.c_str(), m_expno, m_runno, g_nfile);
    g_fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    printf("file %s opened\n", filename);
  }
private:
  int m_fd;
  int m_expno, m_runno;
  std::string m_dir;

};

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : ibufname path_to_disk [obufname] [nodename, nodeid]\n");
    return 1;
  }
  const unsigned interval = 10;
  RunInfoBuffer info;
  const bool use_info = (argc > 7);
  if (use_info) {
    info.open(argv[6], atoi(argv[7]));
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atol(argv[2]) * 1000000, true);
  const std::string dir = argv[3];
  SharedEventBuffer obuf;
  obuf.open(argv[4], atol(argv[5]) * 1000000, true);
  B2INFO("storagerecord: started recording.");
  if (use_info) {
    info.reportRunning();
  }
  int* evtbuf = new int[1000000];
  unsigned long long nbyte_in = 0;
  unsigned long long nbyte_out = 0;
  unsigned int count_in = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  SeqFile* file = NULL;
  int fd = 0;
  while (true) {
    unsigned int nbyte = ibuf.read(evtbuf, true);
    ibuf.lock();
    SharedEventBuffer::Header* iheader = ibuf.getHeader();
    if (expno < iheader->expno || runno < iheader->runno) {
      expno = iheader->expno;
      runno = iheader->runno;
      ibuf.unlock();
      if (use_info) {
        info.setExpNumber(expno);
        info.setRunNumber(runno);
        info.setSubNumber(subno);
        info.setInputCount(0);
        info.setInputNBytes(0);
        info.setOutputCount(0);
        info.setOutputNBytes(0);
        nbyte_in = nbyte_out = 0;
        count_in = count_out = 0;
      }
      obuf.lock();
      SharedEventBuffer::Header* oheader = ibuf.getHeader();
      oheader->expno = expno;
      oheader->runno = runno;
      obuf.unlock();
      char filename[1024];
      sprintf(filename, "%s/e%4.4dr%6.6d.sroot",
              dir.c_str(), expno, runno);
      //if (file != NULL) delete file;
      //file = new SeqFile(filename, "w");
      fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
      printf("file %s opened\n", filename);
      PThread(new FileCloser(0, dir, expno, runno));
      B2INFO("storagerecord : new data file " << filename << " initialized");
    } else {
      ibuf.unlock();
    }
    count_in++;
    nbyte_in += nbyte;
    if (use_info && count_in % 10 == 0) {
      info.setInputCount(count_in);
      info.addInputNBytes(nbyte_in);
    }
    //if (file != NULL) {
    if (fd > 0) {
      //file->write((char*)evtbuf);
      int nbyte = evtbuf[0];
      write(fd, (char*)evtbuf, nbyte);
      int nword = (nbyte - 1) / 4 + 1;
      if (count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword, true);
      }
      count_out++;
      nbyte_out += nbyte;
      if (nbyte_out > MAX_FILE_SIZE) {
        nbyte_out = 0;
        //close(fd);
        //fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
        int fd_c = fd;
        fd = g_fd;
        PThread(new FileCloser(fd_c, dir, expno, runno));
      }
      if (use_info) {
        info.addOutputCount(1);
        info.addOutputNBytes(nbyte);
      }
    } else {
      B2ERROR("storagerecord: no run was initialzed for recording");
    }
    //usleep(1000);
  }
  return 0;
}

