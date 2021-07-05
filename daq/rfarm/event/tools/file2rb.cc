/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "framework/pcore/SeqFile.h"
#include "framework/pcore/RingBuffer.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2rb : rbufname filename neof\n");
    exit(-1);
  }

  SeqFile* file = new SeqFile(argv[2], "r");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }

  //  RingBuffer* rbuf = new RingBuffer(argv[1], RBUFSIZE);
  RingBuffer* rbuf = new RingBuffer(argv[1]);
  rbuf->dump_db();
  char* evbuf = new char[MAXEVTSIZE];

  // Skip the first record (StreamerInfo)
  int is = file->read(evbuf, MAXEVTSIZE);
  if (is <= 0) exit(-1);

  for (;;) {
    int is = file->read(evbuf, MAXEVTSIZE);
    if (is <= 0) break;
    if (is > MAXEVTSIZE) {
      printf("Event size too large : %d\n", is);
      continue;
    }

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbuf->insq((int*)evbuf, (is - 1) / 4 + 1);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
  }
}



