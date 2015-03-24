#ifndef mwreader_h
#define mwreader_h

#define MWMAXITEM 600

const int mwreader_revision = 1;

struct mwreader {
  struct mwdata {
    char unit[8];
    char alarm[8];
    char cond[4];
    int32 chan;
    float value;
    int32 reserved;
  } data[600];
  int32 date;
  int32 time;
  int32 nitem;
};

#endif
