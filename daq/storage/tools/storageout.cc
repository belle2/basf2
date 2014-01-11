//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <stdio.h>
#include <unistd.h>

#include <framework/pcore/RingBuffer.h>
#include <framework/logging/Logger.h>

#include "daq/rfarm/event/RevRb2Sock.h"
#include "daq/dataflow/REvtSocket.h"

#include <daq/storage/BinData.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPServerSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  RunInfoBuffer info;
  bool use_info = (argc > 5);
  if (use_info) info.open(argv[4], atoi(argv[5]));
  RingBuffer* rbuf = new RingBuffer(argv[1]);
  char* evtbuf = new char[10000000];
  int nrec = 0;
  while (true) {
    REvtSocketSend* socket = new REvtSocketSend(atoi(argv[3]));
    info.reportRunning();
    while (true) {
      int size;
      while ((size = rbuf->remq((int*)evtbuf)) == 0) {
        usleep(20);
      }
      EvtMessage* msg = new EvtMessage(evtbuf);
      int is = socket->send(msg);
      delete msg;
      if (is <= 0) {
        B2ERROR("Failed to read data. connection broken.");
        info.reportError();
        break;
      }
      nrec++;
    }
    delete socket;
  }
  return 0;
}

