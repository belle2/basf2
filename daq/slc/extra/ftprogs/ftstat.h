#ifndef _ftstat_h_
#define _ftstat_h_

typedef struct ftstat {
  unsigned int exp;
  unsigned int run;
  unsigned int sub;
  char busy[200];
  char reset[200];
  char stafifo[200];
  unsigned int tincnt;
  unsigned int toutcnt;
  unsigned int atrigc;
  float rateall;
  float raterun;
  float rateout;
  float ratein;
  unsigned int tlimit;
  unsigned int tlast;
  char err[200];
  char errport[200];
  unsigned int errsrc;
  unsigned int errportsrc;
  unsigned int tstart;
  unsigned int trun;
  char state[200];
  char statft[4000];
} ftstat_t;

ftstat_t g_ftstat;

#endif
