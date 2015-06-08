//+
// File : dump_roi.cc
// Description : dump RoI reading data from input stream
//
// Date : 8 - June - 2015
// Author : Ryosuke Itoh, IPNS, KEK
//-

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

enum { OFFSET_LENGTH = 0, OFFSET_HEADER = 1, OFFSET_TRIGNR = 2, OFFSET_RUNNR = 3, OFFSET_ROIS = 4};
enum {
  EXP_MASK = 0xFFC00000,
  EXP_SHIFT = 22,
  RUNNO_MASK = 0x003FFF00,
  RUNNO_SHIFT = 8,
  SUBRUNNO_MASK = 0x000000FF
};


using namespace std;


main(int argc, char** argv)
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
    //    printf ( "nw = %d, 1st = %d, 2nd = %d, 3rd = %d\n",
    //           ntohl(buf[5]), ntohl(buf[6]), ntohl(buf[7]), ntohl(buf[8]) );
    int exp = (ntohl(buf[ 5 + OFFSET_RUNNR ]) & EXP_MASK) >> EXP_SHIFT;
    int run = (ntohl(buf[ 5 + OFFSET_RUNNR ]) & RUNNO_MASK) >> RUNNO_SHIFT;
    int evt = ntohl(buf[ 5 + OFFSET_TRIGNR ]);

    printf("Nrec:%d ; Nw:%d ; Exp:%d, Run:%d, Evt:%d\n",
           nrec, nw, exp, run, evt);
    nrec++;

  }

}
