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

#include <daq/storage/BinData.h>

#include <daq/slc/readout/ProcessStatusBuffer.h>

#include <daq/slc/system/TCPServerSocket.h>

using namespace Belle2;
using namespace std;

#define MAXBUF 100000000

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
  BinData data(new int[MAXBUF]);

  while (true) {
    sbuf.reportReady();
    RingBuffer* rbuf = new RingBuffer(argv[1], 1000000);
    TCPSocket socket;
    try {
      socket = server_socket.accept();
    } catch (const IOException& e) {
      sbuf.reportError("Failed to accept express reco");
      socket.close();
      return 0;
    }
    if (use_buf) sbuf.waitStarted();
    printf("%s:%d\n", __FILE__, __LINE__);
    sbuf.reportRunning();
    printf("%s:%d\n", __FILE__, __LINE__);
    int nrec = 0;
    int size = 0;
    try {
      while (true) {
        bool tried = false;
        while ((size = rbuf->remq(data.getBuffer())) == 0) {
          while (use_buf && sbuf.isStopped()) {
            sbuf.waitStarted();
          }
          //printf("%s:%d\n", __FILE__, __LINE__);
          /*
          if (use_buf) {
            if (!tried) {
              sbuf.reportWarning("Ring buffer empty. waiting to be ready");
            }
            tried = true;
          }
          */
          usleep(20);
        }
        socket.write(data.getBuffer(), data.getWordSize());
        if (nrec % 100 == 0) {
          printf("out record %d: size = %d\n", nrec, data.getWordSize());
        }
        nrec++;
      }
    } catch (const IOException& e) {
      sbuf.reportError("Failed to read data. connection broken.");
    }
    socket.close();
    delete rbuf;
  }
  return 0;
}

