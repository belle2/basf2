#ifndef _Belle2_N6HSLB_h
#define _Belle2_N6HSLB_h

namespace Belle2 {

  class HSLB;

  class N6HSLB {

  private:
    HSLB& m_hslb;

  public:
    N6HSLB(HSLB& hslb);
    ~N6HSLB() {};

  public:
    int write1(unsigned int address, unsigned char v = 0);
    int write2(unsigned int address, unsigned int d);
    int write4(unsigned int address, unsigned int d);
    unsigned char read1(unsigned int address);
    unsigned int read2(unsigned int address);
    unsigned int read4(unsigned int address);

  private:
    int write(unsigned int address, unsigned char v);
    unsigned int read(unsigned int address);

  };

}

#endif
