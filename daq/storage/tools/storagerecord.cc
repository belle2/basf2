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
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : ibufname path_to_disk [obufname] [nodename, nodeid]\n");
    return 1;
  }
  const unsigned interval = 10;
  RunInfoBuffer info;
  storage_info* sinfo = NULL;
  bool use_info = (argc > 4);
  if (use_info) {
    info.open(argv[4], sizeof(storage_info) / sizeof(int), argc > 6);
    sinfo = (storage_info*)info.getReserved();
    sinfo->nodeid = atoi(argv[5]);
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], 25000000, true);
  std::string dir = argv[2];
  SharedEventBuffer obuf;
  obuf.open(argv[3], 10000000, true);
  B2INFO("storagerecord: started recording.");
  info.reportRunning();
  int* evtbuf = new int[1000000];
  unsigned long long datasize = 0;
  unsigned int count = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int count_out = 0;
  SeqFile* file = NULL;
  while (true) {
    ibuf.read(evtbuf);
    ibuf.lock();
    SharedEventBuffer::Header* iheader = ibuf.getHeader();
    if (expno < iheader->expno || runno < iheader->runno) {
      expno = iheader->expno;
      runno = iheader->runno;
      ibuf.unlock();
      if (sinfo != NULL) {
        sinfo->expno = expno;
        sinfo->runno = runno;
        sinfo->subno = 0;
        sinfo->stime = Time().getSecond();
        count = 0;
      }
      obuf.lock();
      SharedEventBuffer::Header* oheader = ibuf.getHeader();
      oheader->expno = expno;
      oheader->runno = runno;
      obuf.unlock();
      char filename[1024];
      sprintf(filename, "%s/e%4.4dr%6.6d.sroot", dir.c_str(), expno, runno);
      if (file != NULL) delete file;
      file = new SeqFile(filename, "w");
      B2INFO("storagerecord : new data file " << filename << " initialized");
    } else {
      ibuf.unlock();
    }
    if (file != NULL) {
      //file->write((char*)evtbuf);
      int nbyte = evtbuf[0];
      int nword = (nbyte - 1) / 4 + 1;
      if (count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword);
      }
      count_out++;
      count++;
      datasize += nbyte;
      if (sinfo != NULL && count % 10 == 0) {
        sinfo->nbyte += datasize;
        sinfo->count = count;
        datasize = 0;
      }
    } else {
      B2ERROR("storagerecord: no run was initialzed for recording");
    }
  }
  return 0;
}

