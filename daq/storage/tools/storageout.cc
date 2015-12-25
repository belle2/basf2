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

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
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
  TCPServerSocket serversocket("0.0.0.0", port);
  try {
    serversocket.open();
  } catch (const IOException& e) {
    B2ERROR("Failed to open server socket 0.0.0.0:" << port);
  }
  try {
    while (true) {
      if (use_info) info.setInputPort(0);
      TCPSocket socket = serversocket.accept();
      TCPSocketWriter writer(socket);
      B2INFO("Connected from expreco.");
      if (use_info) info.setInputPort(port);
      SharedEventBuffer::Header hdr;
      while (true) {
        long long nbyte = ibuf.read(evtbuf, false, &hdr);
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
        try {
          nbyte = writer.write((char*)msg->buffer(), msg->size());
        } catch (const IOException& e) {
          B2WARNING("Lost connection to expreco");
          break;
        }
        //nbyte = socket->send(msg);
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
      B2INFO("Closing sender socket");
      //delete socket;
      socket.close();
    }
  } catch (const std::exception& e) {
    B2ERROR("Unknown error: " << e.what());
  }
  return 0;
}

