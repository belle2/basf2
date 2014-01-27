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
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  RunInfoBuffer info;
  storage_info* sinfo = NULL;
  bool use_info = (argc > 4);
  if (use_info) {
    info.open(argv[4], sizeof(storage_info) / sizeof(int));
    sinfo = (storage_info*)info.getReserved();
    sinfo->nodeid = atoi(argv[5]);
  }
  SharedEventBuffer* buf = new SharedEventBuffer();
  buf->open(argv[1], 25000000, true);
  buf->init();
  TCPSocket socket(argv[2], atoi(argv[3]));
  B2INFO("storagein: Connected to eb2.");
  info.reportRunning();
  int* evtbuf = new int[1000000];
  BinData data;
  data.setBuffer(evtbuf);
  unsigned long long nword = 0;
  Time t0;
  unsigned int count = 0;
  int expno = 0;
  int runno = 0;
  int ntry = 0;
  while (true) {
    try {
      socket.connect();
      socket.setBufferSize(4 * 1024 * 1024);
      ntry = 0;
    } catch (const IOException& e) {
      socket.close();
      B2WARNING("storagein: failed to connect to eb2 (try=" << ntry++ << ")");
      sleep(5);
      continue;
    }
    try {
      TCPSocketReader reader(socket);
      B2INFO("storagein: Cconnected to eb2.");
      while (true) {
        reader.read(evtbuf, sizeof(int));
        reader.read((evtbuf + 1), (evtbuf[0] - 1) * sizeof(int));
        if (expno > data.getExpNumber() || runno > data.getRunNumber()) {
          B2INFO("storagein: old run event detected : exp=" << data.getExpNumber() <<
                 " runno=" << data.getRunNumber() <<
                 " current = (" << expno << "," << runno << ")");
          continue;
        } else if (expno < data.getExpNumber() || runno < data.getRunNumber()) {
          expno = data.getExpNumber();
          runno = data.getRunNumber();
          B2INFO("storagein: new run detected : exp=" << expno << " runno=" << runno);
          if (sinfo != NULL) {
            sinfo->expno = expno;
            sinfo->runno = runno;
            sinfo->subno = 0;
            sinfo->stime = Time().getSecond();
            count = 0;
          }
        }
        buf->write(evtbuf, evtbuf[0]);
        count++;
        nword += data.getWordSize();
        if (sinfo != NULL && count % 1000 == 0) {
          sinfo->evtno = data.getEventNumber();
          sinfo->nword_in = sinfo->nword_out = nword;
          sinfo->count_in = sinfo->count_out = count;
          sinfo->ctime = Time().getSecond();
        }
      }
    } catch (const IOException& e) {
      socket.close();
      B2WARNING("storagein: Connection to eb2 broken.");
      sleep(5);
    }
  }
  return 0;
}

