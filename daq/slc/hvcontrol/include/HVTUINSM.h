#ifndef _Belle2_HVTUINSM_h
#define _Belle2_HVTUINSM_h

#include "daq/slc/hvcontrol/HVTUI.h"
#include "daq/slc/hvcontrol/HVControlCallback.h"

#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"
#include "daq/slc/system/LogFile.h"

#include <list>

//#include <ncurses.h>

namespace Belle2 {

  class NSMCommunicator;

  class HVTUINSM : public HVTUI {

  public:
    HVTUINSM(int crate = -1);
    virtual ~HVTUINSM() throw();

  public:
    void initNSM(const std::string& cfilename,
                 const std::string& cuiname);

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
    virtual void requestState(int crate, int slot, int channel) throw(IOException);
    virtual void requestVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    virtual void requestCurrentMonitor(int crate, int slot, int channel) throw(IOException);
    virtual HVMessage wait() throw(IOException);

  public:
    class HVTUICallback : public HVControlCallback {

    public:
      HVTUICallback(const NSMNode& node, HVTUINSM* tui) throw()
        : HVControlCallback(node), m_tui(tui) {}
      virtual ~HVTUICallback() throw() {}

    public:
      virtual void init(NSMCommunicator&) throw();
      virtual void vset(NSMCommunicator& com, const NSMVar& v) throw()
      {
        NSMVar var = v;
        var.setId(0);
        var.setRevision(0);
        var.setNode("");
        NSMCallback::vset(com, var);
      }

    public:
      virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
      virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
      virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
      virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
      virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
      virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);
      virtual void setState(int crate, int slot, int channel, int state) throw(IOException);
      virtual void setVoltageMonitor(int crate, int slot, int channel, float voltage) throw(IOException);
      virtual void setCurrentMonitor(int crate, int slot, int channel, float current) throw(IOException);
      virtual void timeout(NSMCommunicator&) throw() {}

    private:
      void push(const HVMessage& msg);

    private:
      HVTUINSM* m_tui;

    };

  private:
    std::list<HVMessage> m_msg_l;
    Mutex m_mutex;
    Cond m_cond;
    HVCallback* m_nsm;
    NSMNode m_node;
    int m_max_unit;
    int m_crate;

  };

}

#endif
