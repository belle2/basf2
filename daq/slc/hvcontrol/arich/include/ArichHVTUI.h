#ifndef _Belle2_ArichHVTUI_h
#define _Belle2_ArichHVTUI_h

#include "daq/slc/hvcontrol/HVTUI.h"
#include "daq/slc/hvcontrol/arich/ArichHVCommunicator.h"

#include <list>

#include <ncurses.h>

namespace Belle2 {

  class ArichHVTUI : public HVTUI {

  public:
    ArichHVTUI();
    virtual ~ArichHVTUI() throw();

  public:
    void initialize(const std::string& confname,
                    const std::string& calibname);

  protected:
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);
    virtual void store(int index) throw(IOException);
    virtual void recall(int index) throw(IOException);
    virtual void requestSwitch(int crate, int slot, int channel) throw(IOException);
    virtual void requestRampUpSpeed(int crate, int slot, int channel) throw(IOException);
    virtual void requestRampDownSpeed(int crate, int slot, int channel) throw(IOException);
    virtual void requestVoltageDemand(int crate, int slot, int channel) throw(IOException);
    virtual void requestVoltageLimit(int crate, int slot, int channel) throw(IOException);
    virtual void requestCurrentLimit(int crate, int slot, int channel) throw(IOException);
    virtual void requestVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    virtual void requestCurrentMonitor(int crate, int slot, int channel) throw(IOException);
    virtual void requestState(int crate, int slot, int channel) throw(IOException)
    {
      requestSwitch(crate, slot, channel);
    }
    virtual HVMessage wait() throw(IOException);

  private:
    ArichHVCommunicator m_comm;
    std::list<ArichHVMessage> m_msg_l;
    int m_debug;
    int m_crateid;

  };

}

#endif
