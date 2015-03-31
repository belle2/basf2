#ifndef mwreader_h
#define mwreader_h

#define MWMAXITEM 600

const int mwreader_revision = 2;

struct mwreader {
  struct mwdata {
    int chan;
    float value;
    char unit[8];
    char alarm[8];
    char cond[8];
  } data[MWMAXITEM];
  int date;
  int time;
  int nitem;
};

#endif
