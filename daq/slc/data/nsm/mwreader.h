#ifndef mwreader_h
#define mwreader_h

#define MWMAXITEM 600

const int mwreader_revision = 2;

struct mwreader {
  struct mwdata {
    int32 chan;
    float value;
    char unit[8];
    char alarm[8];
    char cond[8];
  } data[600];
  int32 date;
  int32 time;
  int32 nitem;
};

#endif
