#ifndef DEFREC_H
#define DEFREC_H

enum { EVENT_RECORD_TYPE, BEGIN_RECORD_TYPE, PAUSE_RECORD_TYPE,
       RESUME_RECORD_TYPE, END_RECORD_TYPE
     };

struct EventRec {
  unsigned int type;
  unsigned int len;
  unsigned int evtno;
  unsigned int time;
  unsigned int mstime;
  unsigned int runno; // from run
};

struct BeginRec {
  unsigned int type;
  unsigned int len;
  unsigned int runno;
  unsigned int evtno;
  unsigned int time;
  unsigned int reserve1;
};

struct EndRec {
  unsigned int type;
  unsigned int len;
  unsigned int runno;
  unsigned int evtno;
  unsigned int time;
  unsigned int reserve1;
};

#endif
