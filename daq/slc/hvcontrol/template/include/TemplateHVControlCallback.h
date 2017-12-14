#ifndef _Belle2_TemplateHVControlCallback_h
#define _Belle2_TemplateHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class TemplateHVControlCallback : public HVControlCallback {

  public:
    TemplateHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~TemplateHVControlCallback() throw() {}

  public:
    virtual void initialize(const HVConfig&) throw();
    virtual void update() throw(HVHandlerException);

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

  protected:
    bool m_sw[10][10][100];
    float m_vup[10][10][100];
    float m_vdown[10][10][100];
    float m_vdemand[10][10][100];
    float m_vlim[10][10][100];
    float m_clim[10][10][100];

  };

};

#endif
