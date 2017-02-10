#ifndef _Belle2_ArichHVControlCallback_h
#define _Belle2_ArichHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <vector>

namespace Belle2 {

  class ArichHVControlCallback : public HVControlCallback {

  public:
    ArichHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~ArichHVControlCallback() throw() {}

  public:
    virtual void addAll(const HVConfig& config) throw() ;
    virtual void initialize(const HVConfig& hvconf) throw();
    virtual void deinitialize(int handle) throw();//yone
    virtual void update() throw();
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);//yone
    virtual void standby() throw(HVHandlerException);//yone
    virtual void shoulder() throw(HVHandlerException);//yone
    virtual void peak() throw(HVHandlerException);//yone
    virtual void all_bias_on() throw(HVHandlerException);//yone
    virtual void all_bias_off() throw(HVHandlerException);//yone
    virtual void all_hv_on() throw(HVHandlerException);//yone
    virtual void all_hv_off() throw(HVHandlerException);//yone
    virtual void all_guard_on() throw(HVHandlerException);//yone
    virtual void all_guard_off() throw(HVHandlerException);//yone
    virtual void check_all_bias_on() throw(HVHandlerException);//yone
    virtual void check_all_hv_on() throw(HVHandlerException);//yone
    virtual void check_all_guard_on() throw(HVHandlerException);//yone
    virtual void check_all_bias_off() throw(HVHandlerException);//yone
    virtual void check_all_hv_off() throw(HVHandlerException);//yone
    virtual void check_all_guard_off() throw(HVHandlerException);//yone
    virtual void configure(const HVConfig&) throw(HVHandlerException);

  public:
    virtual void RecoveryTrip(int handle, int crate, int slot, int channel) throw(IOException);
    virtual void RecoveryInterlock(int handle, int crate, int slot, int channel) throw(IOException);
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
    void clearAlarm(int crate) throw(IOException);

  private:
    std::vector<int> m_handle;

  };

};

#endif
