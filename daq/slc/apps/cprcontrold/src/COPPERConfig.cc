#include "daq/slc/apps/cprcontrold/COPPERConfig.h"

using namespace Belle2;

COPPERConfig::COPPERConfig(const COPPERSetup& setup,
                           bool use_hslb_a, bool use_hslb_b,
                           bool use_hslb_c, bool use_hslb_d,
                           const FEEConfig& fee_a,
                           const FEEConfig& fee_b,
                           const FEEConfig& fee_c,
                           const FEEConfig& fee_d,
                           int dmalimit, std::string basf2script,
                           std::string eb0_host, int eb0_port)
{
  m_setup = setup;
  m_use_hslb[0] = use_hslb_a;
  m_use_hslb[1] = use_hslb_b;
  m_use_hslb[2] = use_hslb_c;
  m_use_hslb[3] = use_hslb_d;
  m_fee[0] = fee_a;
  m_fee[1] = fee_b;
  m_fee[2] = fee_c;
  m_fee[3] = fee_d;
  m_dmalimit = dmalimit;
  m_basf2script = basf2script;
  m_eb0_host = eb0_host;
  m_eb0_port = eb0_port;
}

void COPPERConfig::read(const ConfigObject& obj)
{
  m_use_hslb[0] = obj.getBool("hslb_a");
  m_use_hslb[1] = obj.getBool("hslb_b");
  m_use_hslb[2] = obj.getBool("hslb_c");
  m_use_hslb[3] = obj.getBool("hslb_d");
  m_dmalimit = obj.getInt("dmalimit");
  m_basf2script = obj.getText("basf2script");
  m_eb0_host = obj.getText("eb0_host");
  m_eb0_port = obj.getInt("eb0_port");
  m_setup.read(obj.getObject("setup"));
  const ConfigObjectList& fee(obj.getObjects("fee"));
  for (size_t i = 0; i < fee.size(); i++) {
    m_fee[i].read(fee[i]);
  }
}
