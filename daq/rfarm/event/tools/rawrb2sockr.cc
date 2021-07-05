/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

#include <signal.h>

#include "daq/rfarm/event/RawRevRb2Sock.h"

using namespace Belle2;
using namespace std;

static RawRevRb2Sock* s_body = NULL;

void sigint_handler(int signum)
{
  printf("rawrb2sockr : aborted\n");
  delete s_body;
  exit(-1);
}


int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], Shmname[3], and id[4]
  if (argc < 5) {
    printf("syntax : rb2sock rbufname port\n");
    exit(-1);
  }

  signal(SIGINT, sigint_handler);
  signal(SIGTERM, sigint_handler);

  string a1(argv[1]);
  int a2 = atoi(argv[2]);
  string a3(argv[3]);
  int a4 = atoi(argv[4]);

  RawRevRb2Sock* rs = new RawRevRb2Sock(a1, a2, a3, a4);
  //  RawRevRb2Sock rs(a1, a2, a3, a4);
  s_body = rs;

  int nevt = 0;
  for (;;) {
    //    int stat = rs.SendEvent();
    int stat = rs->SendEvent();
    //    printf ( "rb2sock : sending %d\n", stat );
    if (stat <= 0) {
      printf("rb2sock : error in sending event. The event is lost. Reconnecting....\n");
      //      rs.Reconnect();
      rs->Reconnect();
      printf("rb2sock : reconnected.\n");
    }
    nevt++;
    if (nevt % 5000 == 0) {
      printf("rawrb2sockr : evt = %d\n", nevt);
    }
  }
  exit(0);
}



