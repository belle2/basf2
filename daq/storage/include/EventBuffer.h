/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef Belle2_EventBuffer_h
#define Belle2_EventBuffer_h

namespace Belle2 {

  class EventBuffer {

  public:
    struct Header {
      unsigned int expno;
      unsigned int runno;
      unsigned int subno;
      unsigned int count_in;
      unsigned int count_out;
      unsigned long long nword_in;
      unsigned long long nword_out;
      unsigned short nwriter;
      unsigned short nreader;
    };

  public:
    EventBuffer()
    {
      m_buf = 0;
      m_nword = 0;
    }
    EventBuffer(unsigned int nword);
    ~EventBuffer();

  public:
    unsigned int size() throw();
    bool init();
    void clear();

  public:
    Header& getHeader() throw() { return m_header; }
    int* getBuffer() throw() { return m_buf; }
    bool isWritable(int nword) throw();
    bool isReadable() throw();
    unsigned int write(const int* buf, unsigned int nword,
                       unsigned int serial = 0);
    unsigned int read(int* buf, Header* hdr = 0);

  private:
    Header m_header;
    int* m_buf;
    unsigned int m_nword;

  };

}

#endif
