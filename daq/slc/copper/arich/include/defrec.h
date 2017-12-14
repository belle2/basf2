#ifndef DEFREC_H
#define DEFREC_H

enum { RUNREC_ID = 1, ENDREC_ID = 2, POSREC_ID = 3, EVTREC_ID = 4,
       THRREC_ID = 5
     };
enum { MAXDATA = 500};

struct RunRec {
  unsigned int id, len;
  unsigned int time;
  unsigned int fver;
  unsigned int nev, nch, thr, xy;
  int nx, x0, dx, ny, y0, dy;
  int nth, th0, dth;
};

struct EndRec {
  unsigned int id, len;
  unsigned int time;
};

struct PosRec {
  unsigned int id, len;
  unsigned int time;
  int ix, x, xset, iy, y, yset;
};

struct EvtRec {
  unsigned int id, len;
  unsigned int time;
  unsigned int nev;
  unsigned int data[MAXDATA];
};

struct ThrRec {
  unsigned int id, len;
  unsigned int time;
  int ith, th;
};

#endif
