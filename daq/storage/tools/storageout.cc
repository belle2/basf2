//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <stdio.h>
#include <unistd.h>

#include <framework/logging/Logger.h>

#include "daq/rfarm/event/RevRb2Sock.h"
#include "daq/dataflow/REvtSocket.h"

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s bufname bufsize "
                   "port [nodename, nodeid]", argv[0]);
    return 1;
  }
  RunInfoBuffer info;
  storage_info* sinfo = NULL;
  bool use_info = (argc > 5);
  if (use_info) {
    info.open(argv[4], sizeof(storage_info) / sizeof(int), argc > 6);
    sinfo = (storage_info*)info.getReserved();
    sinfo->nodeid = atoi(argv[5]);
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atoi(argv[2]));
  int* evtbuf = new int[1000000];
  unsigned long long datasize = 0;
  unsigned int count = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  info.reportRunning();
  while (true) {
    if (use_info) sinfo->connection = 0;
    REvtSocketSend* socket = new REvtSocketSend(atoi(argv[3]));
    B2INFO("Connected from expreco.");
    if (use_info) sinfo->connection = 1;
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
      } else {
        ibuf.unlock();
      }
      EvtMessage* msg = new EvtMessage((char*)evtbuf);
      int nbyte = socket->send(msg);
      delete msg;
      if (nbyte <= 0) {
        B2WARNING("Connection to expreco broken.");
        if (use_info) sinfo->connection = 0;
        info.reportError();
        break;
      }
      datasize += nbyte;
      count++;
      if (sinfo != NULL && count % 10 == 0) {
        sinfo->count = count;
        sinfo->nbyte += datasize;
        datasize = 0;
      }
    }
    delete socket;
  }
  return 0;
}

