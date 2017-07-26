#ifndef _Belle2_CdcHVControlCallback_h
#define _Belle2_CdcHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class CdcHVControlCallback : public HVControlCallback {

  public:
    CdcHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~CdcHVControlCallback() throw() {}

  public:
    virtual void initialize() throw();
    virtual void timeout() throw();

  public:
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);

    virtual bool getSwitch(int crate, int slot, int channel) throw(IOException);
    virtual float getRampUpSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getRampDownSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageDemand(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageLimit(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentLimit(int crate, int slot, int channel) throw(IOException);
    virtual int getState(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentMonitor(int crate, int slot, int channel) throw(IOException);

    virtual void store(int index) throw(IOException);
    virtual void recall(int index) throw(IOException);

  };

};

#endif
