#ifndef _Belle2_MWReaderImp_h
#define _Belle2_MWReaderImp_h

#include <daq/slc/base/IOException.h>

struct mwreader;

namespace Belle2 {

  class MWReader {

  public:
    MWReader();

  public:
    void set(mwreader* reader);
    mwreader* get() { return m_reader; }

  public:
    void init() throw(IOException);
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

