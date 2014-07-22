//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/slc/readout/ronode_info.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 5) {
    LogFile::debug("%s <nodename> <nodeid> <hostname> <port>", argv[0]);
    return 1;
  }
  RunInfoBuffer info;
  info.open(argv[1], atoi(argv[2]));
  LogFile::info("starting ro dummy.");
  usleep(500000);
  info.reportRunning();

  unsigned long long nbyte = 0;
  unsigned int count = 0;
  int expno = 1;
  int runno = 1;
  int subno = 0;
  info.setInputPort(0);
  info.setInputCount(0);
  info.setInputNBytes(0);
  while (true) {
    TCPSocket socket(argv[3], atoi(argv[4]));
    try {
      socket.connect();
      socket.setBufferSize(4 * 1024 * 1024);
      info.setOutputPort(socket.getLocalPort());
    } catch (const IOException& e) {
      info.setOutputPort(-1);
      socket.close();
      sleep(5);
      continue;
    }
    LogFile::info("connected port=%d", socket.getLocalPort());
    count = 0;
    while (true) {
      try {
        socket.select(0, 0);
      } catch (const IOException& e) {
        info.setOutputPort(-1);
        break;
      }
      info.setExpNumber(expno);
      info.setRunNumber(runno);
      info.setSubNumber(subno);
      info.setOutputCount(count);
      info.addOutputNBytes((int)(rand() / (double)RAND_MAX * 1024000));
      count++;
      usleep(500);
    }
    LogFile::info("disconnected.");
    sleep(5);
  }
  return 0;
}

