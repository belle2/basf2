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
  const std::string host = argv[2];
  TCPServerSocket server_socket;
  server_socket.open(host, atoi(argv[3]));

  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));

  //while (true) {
  sbuf.reportReady();
  RevRb2Sock rs(argv[1], atoi(argv[3]), "DSROUT", -1);
  if (use_buf) sbuf.waitStarted();
  sbuf.reportRunning();
  int nrec = 0;
  while (true) {
    if (rs.SendEvent() <= 0) {
      sbuf.reportError("Failed to read data. connection broken.");
      break;
    }
    if (nrec % 100 == 0) {
      printf("out record %d\n", nrec);
    }
    nrec++;
  }
  //}
  return 0;
}

