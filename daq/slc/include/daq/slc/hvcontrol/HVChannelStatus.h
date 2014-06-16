#ifndef _Belle2_HVChannelStatus_h
#define _Belle2_HVChannelStatus_h

#include "daq/slc/hvcontrol/HVState.h"

#include <string>
#include <vector>

namespace Belle2 {

  class HVChannelStatus {

  public:
    HVChannelStatus();
    virtual ~HVChannelStatus() throw() {}

  public:
    const HVState& getState() const { return m_state; }
    float getVoltageMon() const { return m_voltage_mon; }
    float getCurrentMon() const { return m_current_mon; }
    void setState(const HVState& state) { m_state = state; }
    void setVoltageMon(float voltage) { m_voltage_mon = voltage; }
    void setCurrentMon(float current) { m_current_mon = current; }
    void set(const void* hv_status) throw();
    void get(void* hv_status) const throw();

  private:
    HVState m_state;
    float m_voltage_mon;
    float m_current_mon;

  };

  typedef std::vector<HVChannelStatus> HVChannelStatusList;

}

#endif
