#ifndef _Belle2_CDCHVMessage_h
#define _Belle2_CDCHVMessage_h

#include <string>

namespace Belle2 {

  class CDCHVMessage {

  public:
    enum MessageType {
      GET, SET
    };

    enum Command {
      ALL,
      SWITCH,
      RAMPUP_SPEED,
      RAMPDOWN_SPEED,
      VOLTAGE_DEMAND,
      VOLTAGE_LIMIT,
      CURRENT_LIMIT,
      VOLTAGE_MON,
      CURRENT_MON,
      STORE,
      RECALL,
      LOG,
      DATE
    };

    enum StatusFlag {
      OFF = 0,
      ON = 1,
      OVP = 2,
      OCP = 3
    };

  public:
    CDCHVMessage(MessageType type = GET, Command command = ALL,
                 unsigned short unit = 0, unsigned short ch = 0);
    ~CDCHVMessage() {}

  public:
    std::string toString();
    void read(const std::string& str);

  public:
    void setMessageType(MessageType type) { m_type = type; }
    void setCommand(Command command) { m_command = command; }
    void setUnit(int unit) { m_unit = unit; }
    void setChannel(int ch) { m_ch = ch; }
    void setStatus(StatusFlag v) { m_status = v; }
    void setSwitchOn(bool v) { m_switchon = v; }
    void setRampUpSpeed(unsigned int v) { m_rampup_speed = v; }
    void setRampDownSpeed(unsigned int v) { m_rampdown_speed = v; }
    void setVoltageDemand(unsigned int v) { m_voltage_demand = v; }
    void setVoltageLimit(unsigned int v) { m_voltage_limit = v; }
    void setCurrentLimit(unsigned int v) { m_current_limit = v; }
    void setVoltageMon(unsigned int v) { m_voltage_mon = v; }
    void setCurrentMon(unsigned int v) { m_current_mon = v; }
    void setStoreId(unsigned char v) { m_store = v; }

    MessageType getMessageType() const { return m_type; }
    Command getCommand() const { return m_command; }
    int getUnit() const { return m_unit; }
    int getChannel() const { return m_ch; }
    StatusFlag getStatus() const { return m_status; }
    bool isSwitchOn() const { return m_switchon; }
    unsigned int getVoltageDemand() const { return m_voltage_demand; }
    unsigned int getVoltageLimit() const { return m_voltage_limit; }
    unsigned int getCurrentLimit() const { return m_current_limit; }
    unsigned int getRampUpSpeed() const { return m_rampup_speed; }
    unsigned int getRampDownSpeed() const { return m_rampdown_speed; }
    unsigned int getVoltageMon() const { return m_voltage_mon; }
    unsigned int getCurrentMon() const { return m_current_mon; }
    unsigned char getStoreId() const { return m_store; }

  private:
    MessageType m_type;
    Command m_command;
    unsigned short m_unit, m_ch;
    StatusFlag m_status;
    bool m_switchon;
    unsigned int m_voltage_demand;
    unsigned int m_voltage_limit;
    unsigned int m_current_limit;
    unsigned int m_rampup_speed;
    unsigned int m_rampdown_speed;
    unsigned int m_voltage_mon;
    unsigned int m_current_mon;
    unsigned char m_store;

  };

};

#endif
