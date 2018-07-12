#ifndef _Belle2_ARICHFEB_h
#define _Belle2_ARICHFEB_h

namespace Belle2 {

  class HSLB;

  class ARICHFEB {

  public:
    static void mysleep(int usec);

  protected:
    HSLB& m_hslb;

  public:
    ARICHFEB(HSLB& hslb, int num);
    ~ARICHFEB() {};

  public:
    int write1(unsigned int address, unsigned char v = 0);
    int write2(unsigned int address, unsigned int d);
    int write4(unsigned int address, unsigned int d);
    unsigned char read1(unsigned int address);
    unsigned int read2(unsigned int address);
    unsigned int read4(unsigned int address);
    int num() { return m_num; }

  private:
    int write(unsigned int address, unsigned char v);
    unsigned int read(unsigned int address);

  protected:
    int m_num;

  };

}

#endif
