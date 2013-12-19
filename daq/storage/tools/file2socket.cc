//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include "daq/slc/system/TCPServerSocket.h"

#include "daq/slc/base/Debugger.h"

using namespace Belle2;
using namespace std;

#define MAXBUF  400000000

int main(int argc, char** argv)
{
  if (argc < 4) {
    Belle2::debug("ds_socket2rb : filename hostname port\n");
    return 1;
  }

  int fd = ::open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  TCPServerSocket server_socket;
  const std::string host = argv[2];
  server_socket.open(host, atoi(argv[3]));
  TCPSocket socket = server_socket.accept();
  socket.setBufferSize(32 * 1024 * 1024);
  char* buf = new char[MAXBUF];
  int nrec = 0;
  while (true) {
    int sstat = read(fd, buf, MAXBUF);
    //int sstat = read(fd, buf, sizeof(int));
    if (sstat <= 0) break;
    /*
    int* recsize = (int*)buf;
    int rstat = read(fd, buf + sizeof(int), (*recsize - 1) * 4);
    if (rstat <= 0) break;
    if (nrec % 1000 == 0) {
      printf("record %d: size = %d event = %d\n",
       nrec, recsize[0], recsize[4]);
    }
    socket.write(buf, *recsize * 4);
    */
    socket.write(buf, sstat);
    //usleep(200);
    nrec++;
  }
  socket.close();
  return 0;
}

