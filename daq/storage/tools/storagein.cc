//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <framework/pcore/RingBuffer.h>
#include "daq/rfarm/event/RawRevSock2Rb.h"

#include <daq/storage/BinData.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/base/Debugger.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  RingBuffer* rbuf = new RingBuffer(argv[1], 100000000);
  rbuf->clear();
  Belle2::debug("socket connecting");
  RSocketRecv* socket = new RSocketRecv(argv[2], atoi(argv[3]));
  Belle2::debug("socket connected");
  int* evtbuf = new int[100000000];
  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));
  int nrec = 0;
  bool connected = true;
  BinData data;
  data.setBuffer(evtbuf);
  double datasize = 0;
  Time t0;
  while (true) {
    sbuf.reportRunning();
    while (true) {
      int bufsize = socket->get_wordbuf(evtbuf, 100000000);
      if (bufsize <= 0 && connected) {
        sbuf.reportError("Failed to read data. connection broken.");
        connected = false;
        break;
      } else if (bufsize == 0) {
        break;
      }
      int stat = 0;
      while (true) {
        stat = rbuf->insq(evtbuf, bufsize);
        if (stat >= 0) break;
        usleep(20);
      }
      nrec++;
      datasize += data.getByteSize();
      /*
      if (nrec % 10000 == 0) {
        Time t;
        double freq = 10000. / (t.get() - t0.get()) / 1000. ;
        double rate = datasize / (t.get() - t0.get()) / 1000000.;
        t0 = t;
        datasize = 0;
        std::cout << "Serial = " << nrec << ", Freq = " << freq
                  << " [kHz], Rate = " << rate << " [MB/s], DataSize = "
                  << datasize / 1000. / 1000 << " [kB/event]" << std::endl;
      }
      */
    }
    while (true) {
      if (socket->reconnect(5000) == -1) {
        connected = true;
      } else {
        break;
      }
    }
  }
  return 0;
}

