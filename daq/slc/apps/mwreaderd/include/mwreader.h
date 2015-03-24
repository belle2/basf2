#ifndef mwreader_h
#define mwreader_h

#define MWMAXITEM 600

const int mwreader_revision = 1;

struct mwreader {
  struct mwdata {
    char unit[8];
    char alarm[8];
    char cond[4];
    int chan;
    float value;
    int reserved;
  } data[MWMAXITEM];
  int date;
  int time;
  int nitem;
};

#endif
