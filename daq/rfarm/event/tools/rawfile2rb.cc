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

#include "framework/pcore/RingBuffer.h"

using namespace Belle2;
using namespace std;

#define MAXBUF  400000000

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("rawfile2rb : rbufname filename neof\n");
    exit(-1);
  }

  int fd = open(argv[2], O_RDONLY);
  if (fd < 0) {
    perror("fopen");
    exit(-1);
  }
  int neof = atoi(argv[3]);

  RingBuffer* rbuf = new RingBuffer(argv[1]);
  rbuf->dump_db();

  char* buf = new char[MAXBUF];

  int nrec = 0;

retry:
  for (;;) {
    int sstat = read(fd, buf, sizeof(int));
    if (sstat <= 0) break;
    int* recsize = (int*)buf;
    int rstat = read(fd, buf + sizeof(int), (*recsize - 1) * 4);
    if (rstat <= 0) break;
    if (nrec % 1000 == 0) {
      printf("record %d: size = %d\n", nrec, *recsize);
    }

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbuf->insq((int*)buf, *recsize);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
    nrec++;
  }
  if (neof < 0) {
    lseek(fd, 0, SEEK_SET);
    goto retry;
  }
  close(fd);
}

