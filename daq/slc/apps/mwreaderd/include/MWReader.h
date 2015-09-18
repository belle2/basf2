#ifndef mwreader_h
#define mwreader_h

#define MWMAXITEM 100

const int mwreader_revision = 3;

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
) throw(IOException);
void update() throw(IOException);

private:
void read(int ic, int mode) throw(IOException);

private:
char m_buf[2048];
int m_index;
int* m_sock;
int* m_mw100ip;
mwreader* m_reader;

};

}

#endif

