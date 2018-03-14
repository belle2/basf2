//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

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
      if (use_info) info.setOutputPort(0);
      TCPSocket socket = serversocket.accept();
      TCPSocketWriter writer(socket);
      LogFile::info("Connected from expreco.");
      if (use_info) info.setOutputPort(port);
      SharedEventBuffer::Header hdr;
      while (true) {
        long long nbyte = (ibuf.read(evtbuf, false, false, &hdr)) * sizeof(int);
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
            count_in = count_out = 0;
          }
        }
        count_in++;
        if (use_info) {
          info.addInputCount(1);
          info.addInputNBytes(nbyte);
        }
        unsigned long long nbyte_out = 0;
        try {
          unsigned int nbyte = htonl(evtbuf[0]);
          nbyte_out = writer.write(&nbyte, sizeof(int));
          nbyte_out += writer.write(evtbuf, evtbuf[0]);
          //std::cout << "[DEBUG] In : "<< ntohl(nbyte) << " evtbuf[0] : " << evtbuf[0] << std::endl;
        } catch (const IOException& e) {
          LogFile::warning("Lost connection to expreco %s", e.what());
          break;
        }
        if (nbyte_out <= 0) {
          LogFile::warning("Connection to expreco broken.");
          if (use_info) info.setInputPort(0);
          info.reportError(RunInfoBuffer::SOCKET_OUT);
          break;
        }
        count_out++;
        if (use_info) {
          info.addOutputCount(1);
          info.addOutputNBytes(nbyte_out);
        }
      }
      socket.close();
    }
  } catch (const std::exception& e) {
    B2ERROR("Unknown error: " << e.what());
  }
  return 0;
}

