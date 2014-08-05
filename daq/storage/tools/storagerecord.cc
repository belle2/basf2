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
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Belle2;

const unsigned long long MAX_FILE_SIZE = 2048000000;

class FileHandler {

private:
  FILE* file;
  char* buf;

public:
  void open(const std::string& dir, int expno, int runno) {
    static int g_nfile = 0;
    char filename[1024];
    if (g_nfile > 0) {
      sprintf(filename, "%s/e%4.4dr%6.6d.sroot-%d",
              dir.c_str(), expno, runno, g_nfile);
    } else {
      sprintf(filename, "%s/e%4.4dr%6.6d.sroot",
              dir.c_str(), expno, runno);
    }
    g_nfile++;
    file = fopen(filename, "w");
    if (file == NULL) {
      B2ERROR("failed to open file : " << filename);
    } else {
      buf = (char*)malloc(MAX_FILE_SIZE / 10);
      setvbuf(file, buf, _IOFBF, MAX_FILE_SIZE / 10);
      B2INFO("file " << filename << " opened");
    }
  }

  void close() {
    fclose(file);
    free(buf);
  }

  int write(char* evbuf, int nbyte) {
    return fwrite(evbuf, nbyte, 1, file);
  }

  operator bool() {
    return file != NULL;
  }

};

class FileCloser {

private:
  FileHandler m_handler;

public:
  FileCloser(FileHandler& handler)
    : m_handler(handler) {}

public:
  void run() {
    m_handler.close();
  }

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
  unsigned long long nbyte_out = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  FileHandler file;
  SharedEventBuffer::Header iheader;
  while (true) {
    ibuf.read(evtbuf, true, &iheader);
    int nbyte = evtbuf[0];
    int nword = (nbyte - 1) / 4 + 1;
    if (expno < iheader.expno || runno < iheader.runno) {
      expno = iheader.expno;
      runno = iheader.runno;
      if (use_info) {
        info.setExpNumber(expno);
        info.setRunNumber(runno);
        info.setSubNumber(subno);
        info.setInputCount(0);
        info.setInputNBytes(0);
        info.setOutputCount(0);
        info.setOutputNBytes(0);
        nbyte_out = 0;
        count_out = 0;
      }
      obuf.lock();
      SharedEventBuffer::Header* oheader = ibuf.getHeader();
      oheader->expno = expno;
      oheader->runno = runno;
      obuf.unlock();
      file.open(dir, expno, runno);
    }
    if (use_info) {
      info.addInputCount(1);
      info.addInputNBytes(nbyte);
    }
    if (file) {
      if (nbyte_out > MAX_FILE_SIZE) {
        PThread(new FileCloser(file));
        nbyte_out = 0;
        file.open(dir, expno, runno);
      }
      file.write((char*)evtbuf, nbyte);
      nbyte_out += nbyte;
      if (count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword, true);
      }
      count_out++;
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

