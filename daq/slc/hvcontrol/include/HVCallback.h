#ifndef _Belle2_HVCallback_hh
#define _Belle2_HVCallback_hh

#include "daq/slc/hvcontrol/HVState.h"
#include "daq/slc/hvcontrol/HVMessage.h"
#include "daq/slc/hvcontrol/HVConfig.h"
#include "daq/slc/hvcontrol/HVHandlerException.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <vector>

namespace Belle2 {

  class HVCallback : public NSMCallback {

  protected:
    enum ConfigFlag {
      FLAG_STANDBY = 1, FLAG_SHOULDER = 2, FLAG_PEAK = 3
    };

  public:
    HVCallback() throw();
    virtual ~HVCallback() throw() {}

  public:
    virtual void turnon() throw(HVHandlerException) = 0;
    virtual void turnoff() throw(HVHandlerException) = 0;
    virtual void standby() throw(HVHandlerException) = 0;
    virtual void shoulder() throw(HVHandlerException) = 0;
    virtual void peak() throw(HVHandlerException) = 0;
    virtual void recover() throw(HVHandlerException) {}
    const HVState& getStateDemand() const throw() { return m_state_demand; }
    virtual void store(int /*index*/) throw(IOException) {}
    virtual void recall(int /*index*/) throw(IOException) {}
    virtual void configure(const HVConfig&) throw(HVHandlerException) {}
    virtual void initialize(const HVConfig&) throw() {}

  public:
    virtual void setSwitch(int, int, int , bool) throw(IOException) {}
    virtual void setRampUpSpeed(int, int, int , float) throw(IOException) {}
    virtual void setRampDownSpeed(int, int, int , float) throw(IOException) {}
    virtual void setVoltageDemand(int, int, int , float) throw(IOException) {}
    virtual void setVoltageLimit(int, int, int , float) throw(IOException) {}
    virtual void setCurrentLimit(int, int, int , float) throw(IOException) {}
    virtual void setState(int, int, int , int) throw(IOException) {}
    virtual void setVoltageMonitor(int, int, int , float) throw(IOException) {}
    virtual void setCurrentMonitor(int, int, int , float) throw(IOException) {}

    virtual bool getSwitch(int, int, int) throw(IOException) { return true; }
    virtual float getRampUpSpeed(int, int, int) throw(IOException) { return 0; }
    virtual float getRampDownSpeed(int, int, int) throw(IOException) { return 0; }
    virtual float getVoltageDemand(int, int, int) throw(IOException) { return 0; }
    virtual float getVoltageLimit(int, int, int) throw(IOException) { return 0; }
    virtual float getCurrentLimit(int, int, int) throw(IOException) { return 0; }
    virtual int getState(int, int, int) throw(IOException) { return 0; }
    virtual float getVoltageMonitor(int, int, int) throw(IOException) { return 0; }
    virtual float getCurrentMonitor(int, int, int) throw(IOException) { return 0; }

  public:
    HVConfig& getConfigStandby() throw() { return m_config_standby; }
    HVConfig& getConfigShoulder() throw() { return m_config_shoulder; }
    HVConfig& getConfigPeak() throw() { return m_config_peak; }
    const HVConfig& getConfigStandby() const throw() { return m_config_standby; }
    const HVConfig& getConfigShoulder() const throw() { return m_config_shoulder; }
    const HVConfig& getConfigPeak() const throw() { return m_config_peak; }
    HVConfig& getConfig() throw();
    const HVConfig& getConfig() const throw();
    void setStandbyConfig(const HVConfig& config) throw() { m_config_standby = config; }
    void setShoulderConfig(const HVConfig& config) throw() { m_config_shoulder = config; }
    void setPeakConfig(const HVConfig& config) throw() { m_config_peak = config; }
    void setStandbyConfig(const std::string& name) throw() { m_configname_standby = name; }
    void setShoulderConfig(const std::string& name) throw() { m_configname_shoulder = name; }
    void setPeakConfig(const std::string& name) throw() { m_configname_peak = name; }

  public:
    virtual void addAll(const HVConfig& config) throw();
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    void paramLock() { m_param_mutex.lock(); }
    void paramUnlock() { m_param_mutex.unlock(); }
    void setHVState(const HVState& state);

  protected:
    virtual void dbload(HVConfig& config, const std::string& confignames) throw(IOException) = 0;

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    virtual void load(const HVConfig& config,
                      bool alloff, bool loadpars) throw(HVHandlerException) = 0;

  protected:
    Mutex m_mutex;
    std::string m_configname_standby;
    std::string m_configname_shoulder;
    std::string m_configname_peak;
    Mutex m_param_mutex;

  protected:
    HVState m_state_demand;
    HVConfig m_config_standby;
    HVConfig m_config_shoulder;
    HVConfig m_config_peak;
    ConfigFlag m_config;

  protected:
    struct hvmon {
      int state;
      float vmon;
      float cmon;
      float rampup;
      float rampdown;
      float vdemand;
      float vlimit;
      float climit;
      int sw;
    };
    hvmon m_mon[30][1000];//yone [200]->[1000]
    hvmon m_mon_tmp[30][1000];//yone [200]->[1000]
  };

};

#endif
