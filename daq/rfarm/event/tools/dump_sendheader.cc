/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

enum {
  POS_NWORDS = 0,
  POS_HDR_NWORDS = 1,
  POS_NUM_EVE_NUM_NODES = 2,
  POS_EXP_RUN_NUM = 3,
  POS_EVE_NUM = 4,
  POS_NODE_ID = 5
};


using namespace std;

char* timestamp()
{
  struct timeval tb;
  struct tm*     tp;
  static char buf[256];
  //  struct tm      result;
  gettimeofday(&tb, NULL);
  tp = localtime(&tb.tv_sec);
  sprintf(buf, "%02d:%02d:%02d.%03d ",
          tp->tm_hour, tp->tm_min, tp->tm_sec, (int)(tb.tv_usec / 1000));
  //  printf ( " buf = %s\n", buf );
  return buf;
}



int main(int argc, char** argv)
{
  unsigned int buf[4000000];
  int infn = fileno(stdin);
  printf("infn = %d\n", infn);

  int nrec = 0;
  for (;;) {
    int is = read(infn, &buf[0], 4);
    //    int nw = ntohl(buf[0]);
    int nw = buf[0];
    if (is <= 0) break;
    printf("nw = %d (%8.8x)\n", nw, nw);
    is = read(infn, &buf[1], (nw - 1) * 4);
    if (is <= 0) {
      perror("Error to read input");
      exit(-1);
    }
    printf("buf[0] =%8.8x, buf[1] = %8.8x, buf[nw-1] = %8.8x\n",
           buf[0], buf[1], buf[nw - 1]);
    //    if ( abs(nw)> 1000 ) continue;
    //    for ( int j=0;j<8; j++ ) {
    //      printf ( "%8.8x ", buf[j] );
    //    }
    //    printf ( "\n" );
    //        printf ( "nw = %d, 1st = %d, 2nd = %d, 3rd = %d\n",
    //               ntohl(buf[5]), ntohl(buf[6]), ntohl(buf[7]), ntohl(buf[8]) );
    //    int exp = (ntohl(buf[ OFFSET_RUNNR ]) & EXP_MASK) >> EXP_SHIFT;
    //    int run = (ntohl(buf[ OFFSET_RUNNR ]) & RUNNO_MASK) >> RUNNO_SHIFT;
    //    int evt = ntohl(buf[ OFFSET_TRIGNR ]);
    //    int rois = ntohl(buf[ OFFSET_ROIS] );

    //    printf("%s Nrec: %d ; Nw: %d ; Exp: %d, Run: %d, Evt: %d\n",
    //           timestamp(), nrec, nw, exp, run, evt );
    nrec++;

  }

}
