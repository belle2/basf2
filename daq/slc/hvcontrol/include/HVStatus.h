#ifndef _Belle2_HVStatus_h
#define _Belle2_HVStatus_h

#include "daq/slc/hvcontrol/HVState.h"

#include <string>
#include <vector>

namespace Belle2 {

  class HVStatus {

  public:
    enum State {
      OFF = 0,
      ON = 1,
      OVP = 2,
      OCP = 3,
      ERR = 11
    };

  public:
    HVStatus();
    virtual ~HVStatus() throw() {}

  public:
    State getState() const { return m_state; }
    bool isTurnon() const { return m_state > OFF; }
    float getVoltageMon() const { return m_voltage_mon; }
    float getCurrentMon() const { return m_current_mon; }
    float getReserved(int i) const { return m_reserved[i]; }
    void setState(State state) { m_state = state; }
    void setVoltageMon(float voltage) { m_voltage_mon = voltage; }
    void setCurrentMon(float current) { m_current_mon = current; }
    void setReserved(int i, float reserved) { m_reserved[i] = reserved; }
    void set(const void* hv_status) throw();
    void get(void* hv_status) const throw();

  private:
    State m_state;
    float m_voltage_mon;
    float m_current_mon;
    float m_reserved[4];

  };

  typedef std::vector<HVStatus> HVStatusList;

}

#endif
