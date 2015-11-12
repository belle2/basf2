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
    HVConfigList& getConfigs() throw() { return m_config; }
    const HVConfigList& getConfigs() const throw() { return m_config; }
    HVConfig& getConfig(int index = 0) throw(std::out_of_range) { return m_config[index]; }
    const HVConfig& getConfig(int index = 0) const throw(std::out_of_range) { return m_config[index]; }
    void setConfigNames(const std::string& confignames) throw() { m_confignames = confignames; }

  public:
    void addConfig(const HVConfig& config) throw() { m_config.push_back(config); }
    void addAll(const HVConfig& config) throw();
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  protected:
    virtual void dbload(const std::string& confignames) throw(IOException) = 0;
    void resetConfigs() throw() { m_config = HVConfigList(); }

  public:
    virtual bool perform(NSMCommunicator& com) throw();

  protected:
    virtual void load(const HVConfig& config,
                      bool alloff, bool loadpars) throw(HVHandlerException) = 0;

  protected:
    Mutex m_mutex;
    std::string m_confignames;

  protected:
    HVState m_state_demand;
    HVConfigList m_config;

  };

};

#endif
