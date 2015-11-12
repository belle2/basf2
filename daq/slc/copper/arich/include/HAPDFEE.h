#ifndef _Belle2_HAPDFEE_h
#define _Belle2_HAPDFEE_h

#include "daq/slc/copper/arich/SA0xGlobalParam.h"
#include "daq/slc/copper/arich/SA0xChannelParam.h"

#include <daq/slc/copper/FEE.h>

#include <string>

namespace Belle2 {

  class N6SA0xHSLB;

  class HAPDFEE {

  public:
    HAPDFEE();
    ~HAPDFEE() throw() {}

  public:
    void init(RCCallback& callback, HSLB& hslb);
    void boot(HSLB& hslb, const DBObject& obj);
    void load(HSLB& hslb, const DBObject& obj);

  private:
    int execute(N6SA0xHSLB& sa03, const std::string& cmd,
                unsigned int val, int chip = -1, int ch = -1);
    void load_global(N6SA0xHSLB& sa03, unsigned int chip);
    void load_ch(N6SA0xHSLB& sa03, unsigned int chip, unsigned int ch);

  private:
    SA0xGlobalParam m_globalparam[4];
    SA0xChannelParam m_channelparam[4][36];

  };

}

#endif
