#include "daq/slc/apps/cprcontrold/COPPERSetup.h"

using namespace Belle2;

COPPERSetup::COPPERSetup(int crate, int slot,
                         const std::string& hostname,
                         const std::string& copperid,
                         const std::string& serialid,
                         const std::string& prpmc,
                         const std::string& ttrx,
                         const std::string& ttrx_firmware,
                         const std::string& hslb_a,
                         const std::string& hslb_b,
                         const std::string& hslb_c,
                         const std::string& hslb_d,
                         const std::string& hslb_firmware)
{
  m_crate = crate;
  m_slot = slot;
  m_hostname = hostname;
  m_copperid = copperid;
  m_serialid = serialid;
  m_prpmc = prpmc;
  m_ttrx = ttrx;
  m_ttrx_firmware = ttrx_firmware;
  m_hslb[0] = hslb_a;
  m_hslb[1] = hslb_b;
  m_hslb[2] = hslb_c;
  m_hslb[3] = hslb_d;
  m_hslb_firmware = hslb_firmware;
}

void COPPERSetup::read(const DBObject& obj)
{
  m_runtype = obj.getName();
  m_crate = obj.getInt("crate");
  m_slot = obj.getInt("slot");
  m_hostname = obj.getText("hostname");
  m_copperid = obj.getText("copperid");
  m_serialid = obj.getText("serialid");
  m_prpmc = obj.getText("prpmc");
  m_ttrx = obj.getText("ttrx");
  m_ttrx_firmware = obj.getText("ttrx_firmware");
  m_hslb[0] = obj.getText("hslb_a");
  m_hslb[1] = obj.getText("hslb_b");
  m_hslb[2] = obj.getText("hslb_c");
  m_hslb[3] = obj.getText("hslb_d");
  m_hslb_firmware = obj.getText("hslb_firmware");
}

