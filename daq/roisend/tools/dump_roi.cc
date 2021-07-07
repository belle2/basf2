/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

enum { OFFSET_LENGTH = 0, OFFSET_HEADER = 1, OFFSET_TRIGNR = 2, OFFSET_RUNNR = 3, OFFSET_ROIS = 4};
enum {
  EXP_MASK = 0xFFC00000,
  EXP_SHIFT = 22,
  RUNNO_MASK = 0x003FFF00,
  RUNNO_SHIFT = 8,
  SUBRUNNO_MASK = 0x000000FF
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
  unsigned int buf[400000];
  int infn = fileno(stdin);
  printf("infn = %d\n", infn);

  int nrec = 0;
  for (;;) {
    int is = read(infn, &buf[0], 4);
    int nw = ntohl(buf[0]);
    if (is <= 0) break;
    is = read(infn, &buf[1], (nw - 1) * 4);
    //    printf ( "nw = %d\n", nw );
    //    if ( abs(nw)> 1000 ) continue;
    //    for ( int j=0;j<8; j++ ) {
    //      printf ( "%8.8x ", buf[j] );
    //    }
    //    printf ( "\n" );
    //        printf ( "nw = %d, 1st = %d, 2nd = %d, 3rd = %d\n",
    //               ntohl(buf[5]), ntohl(buf[6]), ntohl(buf[7]), ntohl(buf[8]) );
    int exp = (ntohl(buf[ OFFSET_RUNNR ]) & EXP_MASK) >> EXP_SHIFT;
    int run = (ntohl(buf[ OFFSET_RUNNR ]) & RUNNO_MASK) >> RUNNO_SHIFT;
    int evt = ntohl(buf[ OFFSET_TRIGNR ]);
    //    int rois = ntohl(buf[ OFFSET_ROIS] );

    printf("%s Nrec: %d ; Nw: %d ; Exp: %d, Run: %d, Evt: %d\n",
           timestamp(), nrec, nw, exp, run, evt);
    nrec++;

  }

}
