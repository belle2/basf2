/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <fstream>

#include <daq/storage/BinData.h>

#include "daq/slc/system/TCPServerSocket.h"
#include "daq/slc/system/Time.h"

using namespace Belle2;
using namespace std;

#define MAXBUF  400000000

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("ds_socket2rb : filename hostname port interval\n");
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
  const int ninterval = atoi(argv[4]);
  printf("file2socket::accepting\n");
  TCPSocket socket = server_socket.accept();
  printf("file2socket::accepted\n");
  //socket.setBufferSize(32 * 1024 * 1024);
  //char* buf = new char[MAXBUF];
  int nrec = 0;
  Time t0, t1;
  double datasize = 0;
  int* evtbuf = new int[10000000];
  BinData data;
  //data.setBuffer(evtbuf);
  bool newrun = true;
  while (true) {
    //int sstat = read(fd, data.getBuffer(), sizeof(int));
    int sstat = read(fd, evtbuf, sizeof(int));
    if (sstat <= 0) {
      lseek(fd, 0, SEEK_SET);
      continue;
    }
    if (newrun) {
      newrun = false;
      continue;
    }
    //unsigned int nbyte = data.getByteSize() - sizeof(int);
    //int rstat = read(fd, (data.getBuffer() + 1), nbyte);
    unsigned int nbyte = evtbuf[0];
    int rstat = read(fd, (evtbuf + 1), nbyte - sizeof(int));
    if (rstat <= 0) continue;
    if (sstat + rstat != evtbuf[0]) continue;
    unsigned int nbyte_hton = htonl(nbyte);
    //socket.write(data.getBuffer()+1, nbyte + sizeof(int));
    printf("nbyte = %d\n", nbyte);
    socket.write(&nbyte_hton, sizeof(int));
    socket.write(evtbuf, nbyte);
    nrec++;
    datasize += sstat;
    datasize += rstat;
    const int nth = 10000;
    if (nrec % nth == 0) {
      Time t;
      double dt = (t.get() - t0.get());
      double freq = nth / dt  / 1000.;
      double rate = datasize / dt / 1000. / 1000.;
      printf("Serial = %d Freq = %f [kHz] (%f [kHz]), Rate = %f [MB/s], DataSize = %f [kB/event]\n",
             nrec, freq, (nrec / (t.get() - t1.get()) / 1000), rate, datasize / 1000. / nth);
      t0 = t;
      datasize = 0;
    }
    //if (ninterval > 0 && nrec % ninterval == 0) usleep(1000);
    sleep(1);
  }
  socket.close();
  return 0;
}

