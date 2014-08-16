#ifndef _Belle2_COPPERConfig_h
#define _Belle2_COPPERConfig_h

#include "daq/slc/apps/cprcontrold/FEEConfig.h"
#include "daq/slc/apps/cprcontrold/COPPERSetup.h"

#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class COPPERConfig {

  public:
    COPPERConfig() {}
    COPPERConfig(const COPPERSetup& setup,
                 bool use_hslb_a, bool use_hslb_b,
                 bool use_hslb_c, bool use_hslb_d,
                 const FEEConfig& fee_a,
                 const FEEConfig& fee_b,
                 const FEEConfig& fee_c,
                 const FEEConfig& fee_d,
                 int dmalimit, std::string basf2script,
                 std::string eb0_host, int eb0_port);
    ~COPPERConfig() {}

  public:
    void read(const ConfigObject& obj);
    const std::string& getCopperId() const { return m_setup.getCopperId(); }
    const std::string& getHostname() const { return m_setup.getHostname(); }
    bool useHSLB(int index) const { return m_use_hslb[index]; }
    const COPPERSetup& getSetup() const { return m_setup; }
    COPPERSetup& getSetup() { return m_setup; }
    const FEEConfig& getFEE(int index) const { return m_fee[index]; }
    FEEConfig& getFEE(int index) { return m_fee[index]; }
    int getDMALimit() const { return m_dmalimit; }
    const std::string& getBasf2Script() const { return m_basf2script; }
    const std::string& getEB0Host() const { return m_eb0_host; }
    int getEB0Port() const { return m_eb0_port; }

  private:
    bool m_use_hslb[4];
    COPPERSetup m_setup;
    FEEConfig m_fee[4];
    int m_dmalimit;
    std::string m_basf2script;
    std::string m_eb0_host;
    int m_eb0_port;

  };

}

#endif
