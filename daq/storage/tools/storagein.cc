/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <unistd.h>
#include <cstdlib>
#include <iostream>

#include <framework/logging/Logger.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/base/IOException.h>
#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("%s : bufname bufsize hostname port "
                   "[nodename, nodeid]", argv[0]);
    return 1;
  }

  RunInfoBuffer info;
  bool use_info = (argc > 6);
  if (use_info) {
    info.open(argv[5], atoi(argv[6]));
  }
  SharedEventBuffer ibuf;
  ibuf.open(argv[1], atoi(argv[2]) * 1000000);//, true);
  info.reportReady();
  TCPSocket socket(argv[3], atoi(argv[4]));
  info.reportReady();
  int* evtbuf = new int[10000000];
  BinData data;
  data.setBuffer(evtbuf);
  Time t0;
  int expno = 0;
  int runno = 0;
  int subno = 0;
  int ntried = 0;
  while (true) {
    while (socket.get_fd() <= 0) {
      try {
        socket.connect();
        B2INFO("Connected to data source");
        socket.setBufferSize(32 * 1024 * 1024);
        ntried = 0;
        if (info.isAvailable()) {
          info.setInputPort(socket.getLocalPort());
          info.setInputAddress(socket.getLocalAddress());
        }
        break;
      } catch (const IOException& e) {
        socket.close();
        if (info.isAvailable()) {
          info.setInputPort(0);
          info.setInputAddress(0);
        }
        if (ntried < 5)
          B2WARNING("failed to connect to eb2 (try=" << ntried++ << ")");
        sleep(5);
      }
    }
    info.reportRunning();
    try {
      TCPSocketReader reader(socket);
      B2INFO("storagein: Cconnected to eb2.");
      int count = 0;
      while (true) {
        reader.read(data.getBuffer(), sizeof(int));
        unsigned int nbyte = data.getByteSize() - sizeof(int);
        int nword = data.getWordSize();
        reader.read((data.getBuffer() + 1), nbyte);
        nbyte += sizeof(int);
        if (info.isAvailable()) {
          info.addInputCount(1);
          info.addInputNBytes(nbyte);
        }
        if (expno > data.getExpNumber() || runno > data.getRunNumber()) {
          /*
          B2WARNING("storagein: old run event detected : exp="
                    << data.getExpNumber() << " runno="
                    << data.getRunNumber() << " current = ("
                    << expno << "," << runno << ")");
          */
          continue;
        } else if (expno < data.getExpNumber() || runno < data.getRunNumber()) {
          expno = data.getExpNumber();
          runno = data.getRunNumber();
          B2INFO("new run detected : exp=" << expno << " runno=" << runno);
          SharedEventBuffer::Header* iheader = ibuf.getHeader();
          iheader->expno = expno;
          iheader->runno = runno;
          iheader->subno = subno;
          if (info.isAvailable()) {
            info.setExpNumber(expno);
            info.setRunNumber(runno);
            info.setSubNumber(subno);
            info.setInputCount(0);
            info.setInputNBytes(0);
            info.setOutputCount(0);
            info.setOutputNBytes(0);
          }
          count = 0;
        }
        if (count < 1000000 && (count < 10 || (count > 10 && count < 100 && count % 10 == 0) ||
                                (count > 100 && count < 1000 && count % 100 == 0) ||
                                (count > 1000 && count < 10000 && count % 1000 == 0) ||
                                (count > 10000 && count < 100000 && count % 10000 == 0) ||
                                (count > 100000 && count < 1000000 && count % 100000 == 0))) {
          std::cout << "[DEBUG] Event count = " << count << " nword = " << nword << std::endl;
        }
        count++;
        ibuf.write(data.getBuffer(), nword, true);
        if (info.isAvailable()) {
          info.addOutputCount(1);
          info.addOutputNBytes(nword * sizeof(int));
        }
      }
    } catch (const IOException& e) {
      socket.close();
      if (info.isAvailable()) info.setInputPort(0);
      B2WARNING("Connection to eb2 broken.");
      sleep(5);
    }
  }
  return 0;
}

