#ifndef _Belle2_ARICHMerger_h
#define _Belle2_ARICHMerger_h

#include "daq/slc/copper/arich/SA0xGlobalParam.h"
#include "daq/slc/copper/arich/SA0xChannelParam.h"

#include <daq/slc/copper/FEE.h>

#include <string>

namespace Belle2 {

  class ARICHFEBSA0x;

  class ARICHMerger {

  public:
    ARICHMerger(RCCallback& callback, HSLB& hslb);
    ~ARICHMerger() throw() {}

  public:
    void boot(const DBObject& obj, const DBObject o_feb[6], int used[6]);
    void load(const DBObject& obj, const DBObject o_feb[6], int used[6], const std::string& mode);
    int setThreshold(double th0, double dth, int used[6], int index, const std::string& mode);

  private:
    void load_global(ARICHFEBSA0x& sa03, unsigned int chip);
    void load_ch(ARICHFEBSA0x& sa03, unsigned int chip, unsigned int ch);

    //  private:
  public:
    RCCallback& m_callback;
    HSLB& m_hslb;
    SA0xGlobalParam m_globalparam[4];
    SA0xChannelParam m_channelparam[4][36];

  };

}

#endif
