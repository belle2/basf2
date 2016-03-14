//+
// File : dump_header.cc
// Description : dump SendHeader reading data from input stream
//
// Date : 8 - June - 2015
// Author : Ryosuke Itoh, IPNS, KEK
//-

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

enum {
  POS_NWORDS = 0,
  POS_HDR_NWORDS = 1,
  POS_NUM_EVE_NUM_NODES = 2,
  POS_EXP_RUN_NUM = 3,
  POS_EVE_NUM = 4,
  POS_NODE_ID = 5
};

enum {
  EXP_MASK = 0xFFC00000,
  EXP_SHIFT = 22,
  RUNNO_MASK = 0x003FFF00,
  RUNNO_SHIFT = 8,
  SUBRUNNO_MASK = 0x000000FF
};



main(int argc, char** argv)
{
  unsigned int buf[400000];
  int infn = fileno(stdin);
  printf("infn = %d\n", infn);

  int nrec = 0;
  for (;;) {
    int is = read(infn, &buf[0], 4);
    //    printf ( "buf[0] = %d\n", buf[0] );
    if (is <= 0) break;
    is = read(infn, &buf[1], (buf[0] - 1) * 4);
    //    printf ( "nw = %d, 1st = %d, 2nd = %d, 3rd = %d\n",
    //       buf[0], buf[1], buf[2], buf[3] );
    //    printf ( "exprunnum = %8.8x, evtnum = %8.8x\n", buf[POS_EXP_RUN_NUM],
    //       buf[POS_EVE_NUM] );
    int exp = (buf[ POS_EXP_RUN_NUM ] & EXP_MASK) >> EXP_SHIFT;
    int run = (buf[ POS_EXP_RUN_NUM ] & RUNNO_MASK) >> RUNNO_SHIFT;
    int evt = buf[ POS_EVE_NUM ];

    printf("Nrec:%d ; Nw:%d ; Exp:%d, Run:%d, Evt:%d\n",
           nrec, buf[0], exp, run, evt);
    nrec++;
  }

}
