//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>

#include <framework/pcore/RingBuffer.h>

#include <daq/storage/BinData.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/base/Debugger.h>

using namespace Belle2;
using namespace std;

#define MAXBUF  100000000

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("rawfile2rb : rbufname hostname port [nodename, nodeid]\n");
    return 1;
  }
  ProcessStatusBuffer sbuf;
  bool use_buf = (argc > 5);
  if (use_buf) sbuf.open(argv[4], atoi(argv[5]));

  RingBuffer* rbuf = new RingBuffer(argv[1], 100000000);
  rbuf->dump_db();
  rbuf->clear();

  TCPSocket socket(argv[2], atoi(argv[3]));
  BinData data(new unsigned int[2 * 1024 * 1024]);
  while (true) {
    while (true) {
      try {
        socket.connect();
        break;
      } catch (const IOException& e) {
        socket.close();
        sbuf.reportError("Failed to connect eb2.");
      }
      sleep(1);
    }
    sbuf.reportReady();
    if (use_buf) sbuf.waitStarted();
    sbuf.reportRunning();

    int nrec = 0;
    try {
      while (true) {
        while (use_buf && sbuf.isStopped()) {
          sbuf.waitStarted();
        }
        data.recvEvent(socket);
        ///*
        if (nrec % 1000 == 0) {
          printf("in :record %d: event = %d size = %d %d \n",
                 nrec, data.getEventNumber(), data.getWordSize(), sbuf.getInfo().getExpNumber());
        }
        //*/
        int irb = 0;
        bool tried = false;
        while (true) {
          irb = rbuf->insq(data.getBuffer(), data.getWordSize());
          if (irb >= 0) break;
          /*
            if (!tried) {
            sbuf.reportWarning("Ring buffer full. waiting to be available");
            }
          */
          tried = true;
          usleep(200);
        }
        nrec++;
      }
    } catch (const IOException& e) {
      Belle2::debug(e.what());
      sbuf.reportError("Failed to read data. connection broken.");
    }
    socket.close();
  }
  return 0;
}

