#ifndef _Belle2_ArichHVMessage_h
#define _Belle2_ArichHVMessage_h

#include <daq/slc/base/Reader.h>

#include <string>

namespace Belle2 {

  class ArichHVMessage {

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
      MON,
      STATE,
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
    ArichHVMessage(MessageType type = GET,
                   Command command = ALL,
                   unsigned short unit = 0,
                   unsigned short ch = 0);
    ~ArichHVMessage() {}

  public:
    std::string toString(bool use_ch);
    std::string toReplyText(bool use_ch);
    void read(bool use_ch, Reader& reader) throw(IOException);
    void read(bool use_ch, const std::string& str);

  public:
    void setMessageType(MessageType type) { m_type = type; }
    void setCommand(Command command) { m_command = command; }
    void setUnit(int unit) { m_unit = unit; }
    void setChannel(int channel) { m_ch = channel; }
    void setStatus(StatusFlag v) { m_status = v; }
    void setSwitchOn(bool v) { m_switchon = v; }
    void setRampUpSpeed(double v) { m_rampup_speed = v; }
    void setRampDownSpeed(double v) { m_rampdown_speed = v; }
    void setVoltageDemand(double v) { m_voltage_demand = v; }
    void setVoltageLimit(double v) { m_voltage_limit = v; }
    void setCurrentLimit(double v) { m_current_limit = v; }
    void setVoltageMon(double v) { m_voltage_mon = v; }
    void setCurrentMon(double v) { m_current_mon = v; }
    void setStoreId(unsigned char v) { m_store = v; }

    MessageType getMessageType() const { return m_type; }
    Command getCommand() const { return m_command; }
    int getUnit() const { return m_unit; }
    int getChannel() const { return m_ch; }
    StatusFlag getStatus() const { return m_status; }
    bool getSwitchOn() const { return m_switchon; }
    double getVoltageDemand() const { return m_voltage_demand; }
    double getVoltageLimit() const { return m_voltage_limit; }
    double getCurrentLimit() const { return m_current_limit; }
    double getRampUpSpeed() const { return m_rampup_speed; }
    double getRampDownSpeed() const { return m_rampdown_speed; }
    double getVoltageMon() const { return m_voltage_mon; }
    double getCurrentMon() const { return m_current_mon; }
    unsigned char getStoreId() const { return m_store; }

  private:
    std::string encode(double value);
    double decode(const std::string& value);

  private:
    MessageType m_type;
    Command m_command;
    unsigned short m_unit;
    unsigned short m_ch;
    StatusFlag m_status;
    bool m_switchon;
    double m_voltage_demand;
    double m_voltage_limit;
    double m_current_limit;
    double m_rampup_speed;
    double m_rampdown_speed;
    double m_voltage_mon;
    double m_current_mon;
    unsigned char m_store;

  };

};

#endif
