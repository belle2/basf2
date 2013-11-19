#ifndef Belle2_ArichHVMessage_h
#define Belle2_ArichHVMessage_h

#include "HVChannelInfo.h"

namespace Belle2 {

  class ArichHVMessage {

  public:
    enum Command {
      GET, SET
    };

    enum ParamType {
      ALL,
      SWITCH,
      RAMPUP_SPEED,
      RAMPDOWN_SPEED,
      VOLTAGE_DEMAND,
      VOLTAGE_LIMIT,
      CURRENT_LIMIT,
      VOLTAGE_MON,
      CURRENT_MON,
      DATE
    };

  public:
    ArichHVMessage(Command command = GET, ParamType type = ALL,
                   HVChannelInfo* info = NULL)
      : m_command(command), m_type(type), m_info(info) {}
    ~ArichHVMessage() {}

  public:
    std::string toString();
    void read(const std::string& str);

  public:
    void setCommand(Command command) { m_command = command; }
    void setParamType(ParamType type) { m_type = type; }
    void setChannelInfo(HVChannelInfo* info) { m_info = info; }
    Command setCommand() const { return m_command; }
    ParamType setParamType() const { return m_type; }
    HVChannelInfo* setChannelInfo() { return m_info; }

  private:
    Command m_command;
    ParamType m_type;
    HVChannelInfo* m_info;

  };

};

#endif
