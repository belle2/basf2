#ifndef _Belle2_HVMessage_h
#define _Belle2_HVMessage_h

#include <string>
#include <cstdlib>

namespace Belle2 {

  class HVMessage {
  public:
    enum Type {
      UNKNOWN = 0,
      ALL = 1,
      SWITCH,
      RAMPUP_SPEED,
      RAMPDOWN_SPEED,
      VOLTAGE_DEMAND,
      VOLTAGE_LIMIT,
      CURRENT_LIMIT,
      MON,
      STATE,
      VOLTAGE_MON,
      CURRENT_MON,
      STORE,
      RECALL,
      CHANNEL,
      SLOT
    };
    enum State {
      OFF = 0, // power off,
      ON,      // power on without error
      OCP,     // trip due to over current
      OVP,     // trip due to over voltage
      ERR,     // another error
      RAMPUP,  // ramping up voltage
      RAMPDOWN, // ramping down voltage
      TRIP,     // Trip due to current trip
      ETRIP,    // External trip
      INTERLOCK // Inter lock
    };

  public:
    static Type getMessageType(const char* ptype);
    static State getState(const char* pstate);
    static const std::string getStateText(State state);

  public:
    HVMessage();
    HVMessage(int crate, int slot, int channel,
              int switchon, float rampup,
              float rampdown, float vdemand,
              float vlimit, float climit);
    HVMessage(int crate, int slot, int channel,
              int type, int switchon);
    HVMessage(int index, int type);
    HVMessage(int crate, int slot, int channel,
              bool switchon);
    HVMessage(int crate, int slot, int channel,
              int type, float value);
    HVMessage(int crate, int slot, int channel,
              int state, float vmon, float cmon);
    HVMessage(const HVMessage& msg);
    HVMessage(const int* buf);
    ~HVMessage() throw() {}

  public:
    const HVMessage& operator=(const HVMessage& msg);

  public:
    int getType() const { return *m_type; }
    int getCrate() const { return *m_crate; }
    int getSlot() const { return *m_slot; }
    int getChannel() const { return *m_channel; }
    const float* getParams() const { return m_pars; }
    float* getParams() { return m_pars; }
    int getIndex() const;
    bool getTurnOn() const;
    int getState() const;
    float getRampUpSpeed() const;
    float getRampDownSpeed() const;
    float getVoltageDemand() const;
    float getVoltageLimit() const;
    float getCurrentLimit() const;
    float getVoltageMonitor() const;
    float getCurrentMonitor() const;

  private:
    int* m_crate;
    int* m_slot;
    int* m_channel;
    int* m_type;
    float m_pars[12];

  };

}

#endif
