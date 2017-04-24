//+
// File : file2rb.cc
// Description : Get an event from a SeqRoot file and place it in Rbuf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Apr - 2012
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "framework/pcore/SeqFile.h"
#include "daq/dataflow/REvtSocket.h"

#include "TRandom.h"

#define EVENTINTERVAL 5000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2sock : filename port poisson_freq\n");
    exit(-1);
  }

  // Poisson random number generate
  TRandom rand;

  string filename(argv[1]);
  int port = atoi(argv[2]);
  int pfreq = atoi(argv[3]);

  double minterval = 0.0;
  if (pfreq != 0)
    minterval = 1.0E6 / (double)pfreq;

  // Open file
  SeqFile* file = new SeqFile(filename.c_str(), "r");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }

  // Open EventSocket
  REvtSocketSend* sock = new REvtSocketSend(port);

  // Event Buffer
  char* evbuf = new char[MAXEVTSIZE];

  // Skip the first record (StreamerInfo)
  int is = file->read(evbuf, MAXEVTSIZE);
  if (is <= 0) {
    printf("Error in reading file : %d\n", is);
    exit(-1);
  }

  // Event / time counter
  int nevent = 0;
  struct timeval tnow;
  struct timeval tprev;
  gettimeofday(&tnow, NULL);
  gettimeofday(&tprev, NULL);

  printf("Start event loop\n");
  // Loop for event records
  for (;;) {
    int is = file->read(evbuf, MAXEVTSIZE);
    if (is <= 0) {
      printf("Error in reading file : %d\n", is);
      break;
    }
    if (is > MAXEVTSIZE) {
      printf("Event size too large : %d\n", is);
      continue;
    }

    // Put the message to Socket
    EvtMessage* msg = new EvtMessage(evbuf);    // Ptr copy, no overhead

    if (msg->type() == MSG_TERMINATE) {
      printf("EoF found. Exitting.....\n");
      sock->send(msg);
      delete msg;
      return -1;
    } else {
      int is = sock->send(msg);
      delete msg;
      if (is <= 0) {
        printf("Cannot send event. Exitting\n");
        return -1;
      }
      //    return msg->size();
    }
    if (minterval != 0)
      usleep(rand.Poisson(minterval));

    nevent++;
    if (nevent % EVENTINTERVAL == 0) {
      gettimeofday(&tnow, NULL);
      double delta = (double)((tnow.tv_sec - tprev.tv_sec) * 1000000 +
                              (tnow.tv_usec - tprev.tv_usec));
      double rate = ((double)EVENTINTERVAL) / delta * 1.0E6;
      printf("Event = %8d; Average rate = %7.2f Hz\n", nevent, rate);
      tprev = tnow;
    }
  }
}



