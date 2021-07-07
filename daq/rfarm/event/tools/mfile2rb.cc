/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

#include <stdio.h>
#include <unistd.h>

#include "framework/pcore/SeqFile.h"
#include "framework/pcore/RingBuffer.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000
#define MAXEVT 100000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2rb : rbufname filename nevt\n");
    exit(-1);
  }

  SeqFile* file = new SeqFile(argv[2], "r");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }

  int nevt = atoi(argv[3]);

  //  RingBuffer* rbuf = new RingBuffer(argv[1], RBUFSIZE);
  RingBuffer* rbuf = new RingBuffer(argv[1]);
  rbuf->dump_db();

  // Skip the first record (StreamerInfo)
  char* dumbuf = new char[MAXEVTSIZE];
  int is = file->read(dumbuf, MAXEVTSIZE);
  if (is <= 0) exit(-1);
  delete dumbuf;

  char* evbuf[MAXEVT];
  // Create event buffers
  for (int i = 0; i < nevt; i++) {
    evbuf[i] = new char[MAXEVTSIZE];
    while (true) {
      int is = file->read(evbuf[i], MAXEVTSIZE);
      if (is <= 0) {
        perror("read file");
        exit(-1);
      }
      printf("eventsize = %d\n", is);
      if (is > MAXEVTSIZE) {
        printf("Event size too large : %d\n", is);
        exit(-1);
      }
      if (is > 190000) break;
    }
  }

  int totevt = 0;
  for (;;) {
    int bufno = std::rand() % nevt;

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbuf->insq((int*)evbuf[bufno], (is - 1) / 4 + 1);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
    totevt++;
    if (totevt % 1000 == 0)
      printf("mfile2rb : event = %d (bufno = %d)\n", totevt, bufno);
  }
}



