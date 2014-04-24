#include "daq/slc/hvcontrol/arich/ArichHVMessage.h"

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <cstdlib>

using namespace Belle2;

ArichHVMessage::ArichHVMessage(MessageType type, Command command,
                               unsigned short unit, unsigned short ch)
  : m_type(type), m_command(command), m_unit(unit), m_ch(ch)
{
  m_status = OFF;
  m_switchon = 0;
  m_voltage_demand = 0;
  m_voltage_limit = 0;
  m_current_limit = 0;
  m_rampup_speed = 0;
  m_rampdown_speed = 0;
  m_voltage_mon = 0;
  m_current_mon = 0;
  m_store = 0;
}

std::string ArichHVMessage::toString()
{
  std::stringstream ss;
  ss << "#" << getUnit() << getChannel() << " ";
  if (m_type == GET) {
    switch (m_command) {
      case ALL:            ss << "GET"; break;
      case SWITCH:         ss << "SW?"; break;
      case RAMPUP_SPEED:   ss << "RVU?"; break;
      case RAMPDOWN_SPEED: ss << "RVD?"; break;
      case VOLTAGE_DEMAND: ss << "CH5?"; break;
      case VOLTAGE_LIMIT:  ss << "CH2?"; break;
      case CURRENT_LIMIT:  ss << "CH7?"; break;
      case VOLTAGE_MON:    ss << "MNH1"; break;
      case CURRENT_MON:    ss << "MNH2"; break;
      case STORE:          ss << "STORE?"; break;
      case DATE:           ss << "DATE?"; break;
      default: break;
    }
  } else if (m_type == SET) {
    switch (m_command) {
      case ALL: ss << StringUtil::form("SET %04X,%d,%04X,%04X,%04X", getVoltageDemand(),
                                         (int)isSwitchOn(), getVoltageLimit(), getCurrentLimit(),
                                         getRampUpSpeed(), getRampDownSpeed()); break;
      case SWITCH:         ss << "SW" << (int)isSwitchOn(); break;
      case RAMPUP_SPEED:   ss << StringUtil::form("RVU %04X", getRampUpSpeed()); break;
      case RAMPDOWN_SPEED: ss << StringUtil::form("RVD %04X", getRampDownSpeed()); break;
      case VOLTAGE_DEMAND: ss << StringUtil::form("CH5 %04X", getVoltageDemand()); break;
      case VOLTAGE_LIMIT:  ss << StringUtil::form("CH2 %04X", getVoltageLimit()); break;
      case CURRENT_LIMIT:  ss << StringUtil::form("CH7 %04X", getCurrentLimit()); break;
      case STORE:          ss << StringUtil::form("STORE %d", getStoreId()); break;
      case RECALL:         ss << StringUtil::form("RECALL %d", getStoreId()); break;
      default: break;
    }
  }
  return ss.str();
}

void ArichHVMessage::read(const std::string& str)
{
  if (str.size() == 0) return;
  std::string s = str;
  if (str.at(0) == '#') {
    setUnit(str.at(1) - '0');
    setChannel(str.at(2) - '0');
    s = s.substr(s.find(" ") + 1);
  } else {
    setUnit(0);
    setChannel(0);
  }
  s = StringUtil::toupper(s);
  std::vector<std::string> s_v = StringUtil::split(s, ' ');
  if (s_v.size() > 1 || s.find("SW1") != std::string::npos
      || s.find("SW0") != std::string::npos) {
    m_type = SET;
  } else {
    m_type = GET;
  }
  if (m_type == GET) {
    s_v = StringUtil::split(s, '=');
    if (s_v[0].find("GET") != std::string::npos) {
      m_command = ALL;
      if (s_v.size() > 1) {
        std::vector<std::string> ss_v = StringUtil::split(s_v[1], ',');
        setVoltageMon(strtoul(("0x" + ss_v[0]).c_str(), 0, 0));
        setCurrentMon(strtoul(("0x" + ss_v[1]).c_str(), 0, 0));
        setStatus((StatusFlag)atoi(ss_v[2].c_str()));
        setSwitchOn(ss_v[2] == "1");
        setRampUpSpeed(strtoul(("0x" + ss_v[3]).c_str(), 0, 0));
        setRampDownSpeed(strtoul(("0x" + ss_v[4]).c_str(), 0, 0));
      }
    } else if (s_v[0].find("SW") != std::string::npos) {
      m_command = SWITCH;
      if (s_v.size() > 1) {
        setSwitchOn(s_v[1] != "0");
        setStatus((StatusFlag)atoi(s_v[1].c_str()));
      }
    } else if (s_v[0].find("RVU") != std::string::npos) {
      m_command = RAMPUP_SPEED;
      if (s_v.size() > 1) setRampUpSpeed(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("RVD") != std::string::npos) {
      m_command = RAMPDOWN_SPEED;
      if (s_v.size() > 1) setRampDownSpeed(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH5") != std::string::npos) {
      m_command = VOLTAGE_DEMAND;
      if (s_v.size() > 1) setVoltageDemand(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH2") != std::string::npos) {
      m_command = VOLTAGE_LIMIT;
      if (s_v.size() > 1) setVoltageLimit(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH7") != std::string::npos) {
      m_command = CURRENT_LIMIT;
      if (s_v.size() > 1) setCurrentLimit(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("MNH1") != std::string::npos) {
      m_command = VOLTAGE_MON;
      if (s_v.size() > 1) setVoltageMon(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("MNH2") != std::string::npos) {
      m_command = CURRENT_MON;
      if (s_v.size() > 1) setCurrentMon(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("DATE") != std::string::npos) {
      m_command = DATE;
    } else if (s_v[0].find("RECALL") != std::string::npos) {
      m_command = RECALL;
      if (s_v.size() > 1) setStoreId(atoi(s_v[1].c_str()));
    } else if (s_v[0].find("STORE") != std::string::npos) {
      m_command = STORE;
      if (s_v.size() > 1) setStoreId(atoi(s_v[1].c_str()));
    }
  } else if (m_type == SET) {
    if (s_v[0].find("SW0") != std::string::npos) {
      m_command = SWITCH;
      setSwitchOn(false);
    } else if (s_v[0].find("SW1") != std::string::npos) {
      m_command = SWITCH;
      setSwitchOn(true);
    } else if (s_v.size() > 1) {
      std::string s = s_v[1];
      if (s_v[0].find("SET") != std::string::npos) {
        m_command = ALL;
        std::vector<std::string> ss_v = StringUtil::split(s, ',');
        if (ss_v.size() > 5) {
          setVoltageDemand(strtoul(("0x" + ss_v[0]).c_str(), 0, 0));
          setSwitchOn(ss_v[1] == "1");
          setVoltageLimit(strtoul(("0x" + ss_v[2]).c_str(), 0, 0));
          setCurrentLimit(strtoul(("0x" + ss_v[3]).c_str(), 0, 0));
          setRampUpSpeed(strtoul(("0x" + ss_v[4]).c_str(), 0, 0));
          setRampDownSpeed(strtoul(("0x" + ss_v[5]).c_str(), 0, 0));
        }
      } else if (s_v[0].find("RVU") != std::string::npos) {
        m_command = RAMPUP_SPEED;
        setRampUpSpeed(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0].find("RVD") != std::string::npos) {
        m_command = RAMPDOWN_SPEED;
        setRampDownSpeed(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0].find("CH5") != std::string::npos) {
        m_command = VOLTAGE_DEMAND;
        setVoltageDemand(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0].find("CH2") != std::string::npos) {
        m_command = VOLTAGE_LIMIT;
        setVoltageLimit(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0].find("CH7") != std::string::npos) {
        m_command = CURRENT_LIMIT;
        setCurrentLimit(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0].find("RECALL") != std::string::npos) {
        m_command = RECALL;
        setStoreId(atoi(s.c_str()));
      } else if (s_v[0].find("STORE") != std::string::npos) {
        m_command = STORE;
        setStoreId(atoi(s.c_str()));
      } else if (s_v[0].find("DATE") != std::string::npos) {
        m_command = DATE;
      }
    }
  }
}

