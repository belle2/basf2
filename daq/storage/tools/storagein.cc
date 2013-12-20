//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>

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
  RawRevSock2Rb sr(argv[1], argv[2], atoi(argv[3]), "DSRIN", -1);
  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));
  int nrec = 0;
  while (true) {
    sbuf.reportReady();
    if (use_buf) sbuf.waitStarted();
    sbuf.reportRunning();
    while (true) {
      while (use_buf && sbuf.isStopped()) sbuf.waitStarted();
      if (sr.ReceiveEvent() < 0) {
        sbuf.reportError("Failed to read data. connection broken.");
        break;
      } else if (sr.ReceiveEvent() == 0) {
        break;
      }
      nrec++;
      if (nrec % 1000 == 0) printf("in :record %d\n", nrec);
    }
    while (true) {
      if (sr.Reconnect(5000) == -1) {
        //sbuf.reportError("Reconnection failed");
      } else {
        break;
      }
    }
  }
  return 0;
}

