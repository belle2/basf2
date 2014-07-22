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

using namespace Belle2;

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
  ibuf.open(argv[1], atoi(argv[2]), true);
  const std::string dir = argv[3];
  SharedEventBuffer obuf;
  obuf.open(argv[4], atoi(argv[5]), true);
  B2INFO("storagerecord: started recording.");
  info.reportRunning();
  int* evtbuf = new int[1000000];
  unsigned long long nbyte_in = 0;
  unsigned long long nbyte_out = 0;
  unsigned int count_in = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  SeqFile* file = NULL;
  while (true) {
    unsigned int nbyte = ibuf.read(evtbuf);
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
      sprintf(filename, "%s/e%4.4dr%6.6d.sroot", dir.c_str(), expno, runno);
      if (file != NULL) delete file;
      file = new SeqFile(filename, "w");
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
    if (file != NULL) {
      file->write((char*)evtbuf);
      int nbyte = evtbuf[0];
      int nword = (nbyte - 1) / 4 + 1;
      if (count_out % interval == 0 && obuf.isWritable(nword)) {
        obuf.write(evtbuf, nword);
      }
      count_out++;
      nbyte_out += nbyte;
      if (use_info && count_out % 10 == 0) {
        info.setOutputCount(count_out);
        info.addOutputNBytes(nbyte_out);
        nbyte_out = 0;
      }
    } else {
      B2ERROR("storagerecord: no run was initialzed for recording");
    }
  }
  return 0;
}

