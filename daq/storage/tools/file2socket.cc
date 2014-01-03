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
#include "daq/slc/system/Time.h"

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

  printf("file2socket::start\n");
  TCPServerSocket server_socket;
  const std::string host = argv[2];
  server_socket.open(host, atoi(argv[3]));
  printf("file2socket::accepting\n");
  TCPSocket socket = server_socket.accept();
  printf("file2socket::accepted\n");
  socket.setBufferSize(32 * 1024 * 1024);
  char* buf = new char[MAXBUF];
  int nrec = 0;
  Time t0;
  double datasize = 0;
  while (true) {
    int sstat = read(fd, buf, MAXBUF);
    //int sstat = read(fd, buf, sizeof(int));
    if (sstat <= 0) {
      lseek(fd, 0, SEEK_SET);
      continue;
    }
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
    usleep(50);
    nrec++;
    datasize += sstat;
    if (nrec % 10000 == 0) {
      Time t;
      double freq = 10000. / (t.get() - t0.get()) / 1000. ;
      double rate = datasize / (t.get() - t0.get()) / 1000000.;
      printf("Serial = %d Freq = %f [kHz], Rate = %f [MB/s], DataSize = %f [kB/event]\n",
             nrec, freq, rate, datasize / 1000. / 1000);
      t0 = t;
      datasize = 0;
    }
  }
  socket.close();
  return 0;
}

