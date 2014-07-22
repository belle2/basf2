//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <framework/logging/Logger.h>

#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

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
  ibuf.open(argv[1], atoi(argv[2]) * 1000000, true);
  TCPSocket socket(argv[3], atoi(argv[4]));
  B2INFO("storagein: Connected to eb2.");
  info.reportRunning();
  int* evtbuf = new int[1000000];
  BinData data;
  data.setBuffer(evtbuf);
  Time t0;
  unsigned long long nbyte_in = 0;
  unsigned int count_in = 0;
  unsigned long long nbyte_out = 0;
  unsigned int count_out = 0;
  int expno = 0;
  int runno = 0;
  int subno = 0;
  int ntried = 0;
  while (true) {
    try {
      socket.connect();
      socket.setBufferSize(4 * 1024 * 1024);
      ntried = 0;
      if (info.isAvailable()) {
        info.setInputPort(socket.getLocalPort());
      }
    } catch (const IOException& e) {
      socket.close();
      if (info.isAvailable()) info.setInputPort(0);
      B2WARNING("storagein: failed to connect to eb2 (try=" << ntried++ << ")");
      sleep(5);
      continue;
    }
    try {
      TCPSocketReader reader(socket);
      B2INFO("storagein: Cconnected to eb2.");
      while (true) {
        reader.read(evtbuf, sizeof(int));
        unsigned int nbyte = (evtbuf[0] - 1) * sizeof(int);
        reader.read((evtbuf + 1), nbyte);
        nbyte_in += nbyte;
        if (info.isAvailable() && count_in % 10 == 0) {
          info.setInputCount(count_in);
          info.addInputNBytes(nbyte_in);
          nbyte_in = 0;
        }
        if (expno > data.getExpNumber() || runno > data.getRunNumber()) {
          B2INFO("storagein: old run event detected : exp="
                 << data.getExpNumber() << " runno="
                 << data.getRunNumber() << " current = ("
                 << expno << "," << runno << ")");
          continue;
        } else if (expno < data.getExpNumber() || runno < data.getRunNumber()) {
          expno = data.getExpNumber();
          runno = data.getRunNumber();
          B2INFO("storagein: new run detected : exp="
                 << expno << " runno=" << runno);
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
            nbyte_in = nbyte;
            nbyte_out = 0;
            count_in = 1;
            count_out = 0;
          }
        }
        nbyte_out += ibuf.write(evtbuf, evtbuf[0]);
        count_out++;
        data.getByteSize();
        if (info.isAvailable() && count_out % 10 == 0) {
          info.setOutputCount(count_out);
          info.addOutputNBytes(nbyte_out);
          nbyte_out = 0;
        }
      }
    } catch (const IOException& e) {
      socket.close();
      if (info.isAvailable()) info.setInputPort(0);
      B2WARNING("storagein: Connection to eb2 broken.");
      sleep(5);
    }
  }
  return 0;
}

