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
#include "daq/rfarm/event/RawRevSock2Rb.h"

#include <daq/storage/BinData.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  RingBuffer* rbuf = new RingBuffer(argv[1], 100000000);
  rbuf->clear();
  Belle2::debug("socket connecting");
  RSocketRecv* socket = new RSocketRecv(argv[2], atoi(argv[3]));
  Belle2::debug("socket connected");
  int* evtbuf = new int[100000000];
  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));
  int nrec = 0;
  while (true) {
    sbuf.reportReady();
    //if (use_buf) sbuf.waitStarted();
    //sbuf.reportRunning();
    while (true) {
      //while (use_buf && sbuf.isStopped()) sbuf.waitStarted();
      int bufsize = socket->get_wordbuf(evtbuf, 100000000);
      if (bufsize <= 0) {
        sbuf.reportError("Failed to read data. connection broken.");
        break;
      } else if (bufsize == 0) {
        break;
      }
      int stat = 0;
      while (true) {
        stat = rbuf->insq(evtbuf, bufsize);
        if (stat >= 0) break;
        usleep(20);
      }
      nrec++;
      if (nrec % 10000 == 0) printf("in :record %d\n", nrec);
    }
    while (true) {
      if (socket->reconnect(5000) == -1) {
      } else {
        break;
      }
    }
  }
  return 0;
}

