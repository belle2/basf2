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

#include "daq/rfarm/event/RevRb2Sock.h"
#include "daq/dataflow/EvtSocket.h"

#include <daq/storage/BinData.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/system/TCPServerSocket.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));

  //while (true) {
  sbuf.reportReady();
  RingBuffer* rbuf = new RingBuffer(argv[1]);
  char* evtbuf = new char[10000000];
  //RevRb2Sock rs(argv[1], atoi(argv[3]), "DSROUT", -1);
  if (use_buf) sbuf.waitStarted();
  sbuf.reportRunning();
  int nrec = 0;
  while (true) {
    EvtSocketSend* socket = new EvtSocketSend(argv[2], atoi(argv[3]));
    while (true) {
      int size;
      while ((size = rbuf->remq((int*)evtbuf)) == 0) {
        usleep(20);
      }
      EvtMessage* msg = new EvtMessage(evtbuf);
      if (msg->type() == MSG_TERMINATE) {
        printf("EoF found. Exitting.....\n");
        socket->send(msg);
        delete msg;
        return -1;
      } else {
        int is = socket->send(msg);
        delete msg;
        sbuf.reportError("Failed to read data. connection broken.");
        break;
      }
      if (nrec % 100 == 0) {
        printf("out record %d\n", nrec);
      }
      nrec++;
    }
    delete socket;
  }
  return 0;
}

