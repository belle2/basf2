#ifndef _Belle2_COPPERSetup_h
#define _Belle2_COPPERSetup_h

#include <daq/slc/database/DBObject.h>

#include <string>

namespace Belle2 {

  class COPPERSetup {

  public:
    COPPERSetup() {}
    COPPERSetup(int crate, int slot,
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
                const std::string& hslb_firmware);
    ~COPPERSetup() {}

  public:
    int getCrate() const { return m_crate; }
    int getSlot() const { return m_slot; }
    const std::string& getRunType() const { return m_runtype; }
    const std::string& getHostname() const { return m_hostname; }
    const std::string& getCopperId() const { return m_copperid; }
    const std::string& getSerialId() const { return m_serialid; }
    const std::string& getPrPMC() const { return m_prpmc; }
    const std::string& getTTRX() const { return m_ttrx; }
    const std::string& getTTRXFirmware() const { return m_ttrx_firmware; }
    const std::string& getHSLB(int i) const { return m_hslb[i]; }
    const std::string& getHSLBFirmware() const { return m_hslb_firmware; }
    void read(const DBObject& obj);

  private:
    int m_crate;
    int m_slot;
    std::string m_runtype;
    std::string m_hostname;
    std::string m_copperid;
    std::string m_serialid;
    std::string m_prpmc;
    std::string m_ttrx;
    std::string m_ttrx_firmware;
    std::string m_hslb[4];
    std::string m_hslb_firmware;

  };

}

#endif
