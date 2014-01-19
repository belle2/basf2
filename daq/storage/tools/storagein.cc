//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <framework/pcore/RingBuffer.h>
#include <framework/logging/Logger.h>
#include "daq/rfarm/event/RawRevSock2Rb.h"

#include <daq/storage/BinData.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
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
  bool use_info = (argc > 4);
  if (use_info) info.open(argv[4]);
  RingBuffer* rbuf = new RingBuffer(argv[1], 100000000);
  rbuf->cleanup();
  delete rbuf;
  rbuf = new RingBuffer(argv[1], 100000000);
  RSocketRecv* socket = new RSocketRecv(argv[2], atoi(argv[3]));
  B2INFO("storagein: Connected to eb2.");
  info.reportRunning();
  int* evtbuf = new int[100000000];
  int nrec = 0;
  bool connected = true;
  BinData data;
  data.setBuffer(evtbuf);
  double datasize = 0;
  Time t0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int evtno = 0;
  while (true) {
    while (true) {
      int bufsize = socket->get_wordbuf(evtbuf, 100000000);
      if (bufsize <= 0 && connected) {
        B2WARNING("storagein: Connection to eb2 broken.");
        connected = false;
        break;
      } else if (bufsize == 0) {
        break;
      }
      if (expno > data.getExpNumber() || runno > data.getRunNumber()) {
        B2INFO("old run event detected : exp=" << data.getExpNumber() << " runno=" << data.getRunNumber() <<
               " current = (" << expno << "," << runno << ")");
        continue;
      } else if (expno < data.getExpNumber() || runno < data.getRunNumber()) {
        expno = data.getExpNumber();
        runno = data.getRunNumber();
      }
      int stat = 0;
      while (true) {
        stat = rbuf->insq(evtbuf, bufsize);
        if (stat >= 0) break;
        usleep(20);
      }
      nrec++;
      datasize += data.getByteSize();
    }
    while (true) {
      if (socket->reconnect(5000) == -1) {
        connected = false;
      } else {
        info.reportRunning();
        connected = true;
        break;
      }
    }
    B2INFO("storagein: Reconnected to eb2.");
  }
  return 0;
}

