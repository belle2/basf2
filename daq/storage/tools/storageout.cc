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
  bool use_info = (argc > 5);
  if (use_info) {
    info.open(argv[4], atoi(argv[5]));
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atoi(argv[2]) * 1000000);
  int* evtbuf = new int[10000000];
  unsigned long long nbyte_in = 0;
  unsigned long long nbyte_out = 0;
  unsigned int count_in = 0;
  unsigned int count_out = 0;
  unsigned int expno = 0;
  unsigned int runno = 0;
  unsigned int subno = 0;
  info.reportRunning();
  const int port = atoi(argv[3]);
  while (true) {
    if (use_info) info.setInputPort(0);
    REvtSocketSend* socket = new REvtSocketSend(port);
    B2INFO("Connected from expreco.");
    if (use_info) {
      info.setInputPort(port);
    }
    SharedEventBuffer::Header hdr;
    while (true) {
      unsigned int nbyte = ibuf.read(evtbuf, false, &hdr);
      if (expno < hdr.expno || runno < hdr.runno) {
        expno = hdr.expno;
        runno = hdr.runno;
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
      }
      count_in++;
      nbyte_in += nbyte;
      if (use_info && count_in % 10 == 0) {
        info.setInputCount(count_in);
        info.addInputNBytes(nbyte_in);
      }
      EvtMessage* msg = new EvtMessage((char*)evtbuf);
      nbyte = socket->send(msg);
      delete msg;
      if (nbyte <= 0) {
        B2WARNING("Connection to expreco broken.");
        if (use_info) info.setInputPort(0);
        info.reportError(RunInfoBuffer::SOCKET_OUT);
        break;
      }
      nbyte_out += nbyte;
      count_out++;
      if (use_info && count_out % 10 == 0) {
        info.setOutputCount(count_out);
        info.addOutputNBytes(nbyte_out);
        nbyte_out = 0;
      }
    }
    delete socket;
  }
  return 0;
}

