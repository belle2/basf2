#include "daq/slc/hvcontrol/arich/ArichHVMessage.h"

#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace Belle2;

std::string ArichHVMessage::encode(double value)
{
  return StringUtil::form("%04X", (unsigned int)value);
}

double ArichHVMessage::decode(const std::string& value)
{
  return strtoul(("0x" + value).c_str(), 0, 0);
}

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

std::string ArichHVMessage::toString(bool use_ch)
{
  std::stringstream ss;
  if (use_ch) {
    ss << "#" << getUnit() << getChannel() << " ";
  } else {
    ss << "#" << getUnit() << " ";
  }
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
      case ALL: ss << "SET " << encode(getVoltageDemand())
                     << "," << (int)getSwitchOn()
                     << "," << encode(getVoltageLimit())
                     << "," << encode(getCurrentLimit())
                     << "," << encode(getRampUpSpeed())
                     << "," << encode(getRampDownSpeed()); break;
      case SWITCH:         ss << "SW" << (int)getSwitchOn(); break;
      case RAMPUP_SPEED:   ss << "RVU " << encode(getRampUpSpeed()); break;
      case RAMPDOWN_SPEED: ss << "RVD " << encode(getRampDownSpeed()); break;
      case VOLTAGE_DEMAND: ss << "CH5 " << encode(getVoltageDemand()); break;
      case VOLTAGE_LIMIT:  ss << "CH2 " << encode(getVoltageLimit()); break;
      case CURRENT_LIMIT:  ss << "CH7 " << encode(getCurrentLimit()); break;
      case STORE:          ss << "STORE " << (int)getStoreId(); break;
      case RECALL:         ss << "RECALL " << (int)getStoreId(); break;
      default: break;
    }
  }
  return ss.str();
}

std::string ArichHVMessage::toReplyText(bool use_ch)
{
  std::stringstream ss;
  if (use_ch) {
    ss << "#" << getUnit() << getChannel();
  } else {
    ss << "#" << getUnit();
  }
  switch (getCommand()) {
    case ALL: {
      ss << "GET="
         << encode(getVoltageMon()) << ","
         << encode(getCurrentMon()) << ","
         << getSwitchOn() << ","
         << encode(getRampUpSpeed()) << ","
         << encode(getRampDownSpeed());
    } break;
    case SWITCH:  ss << "SW=" << (int)getSwitchOn(); break;
    case RAMPUP_SPEED: ss << "RVU=" << encode(getRampUpSpeed()); break;
    case RAMPDOWN_SPEED: ss << "RVD=" << encode(getRampDownSpeed()); break;
    case VOLTAGE_DEMAND: ss << "CH5=" << encode(getVoltageDemand()); break;
    case VOLTAGE_LIMIT: ss << "CH2=" << encode(getVoltageLimit()); break;
    case CURRENT_LIMIT: ss << "CH7=" << encode(getCurrentLimit()); break;
    case VOLTAGE_MON: ss << "MNH1=" << encode(getVoltageMon()); break;
    case CURRENT_MON: ss << "MNH2=" << encode(getCurrentMon()); break;
    default: break;
  }
  return ss.str();
}

void ArichHVMessage::read(bool use_ch, Reader& reader) throw(IOException)
{
  std::stringstream ss;
  while (true) {
    char c = reader.readChar();
    if (c == '\r') break;
    ss << c;
  }
  read(use_ch, ss.str());
}

void ArichHVMessage::read(bool use_ch, const std::string& str)
{
  if (str.size() == 0 || str.at(0) != '#') return;
  std::string s = str;
  std::stringstream ss;
  int i = 1;
  if (use_ch) {
    setUnit(s.at(1) - '0');
    setChannel(s.at(2) - '0');
    i = 3;
  } else {
    for (; i < (int)str.size(); i++) {
      if (!isdigit(s.at(i))) break;
      ss << s.at(i);
    }
    setUnit(atoi(ss.str().c_str()));
    setChannel(0);
  }
  s = s.substr(i);
  if (s.at(0) == ' ') s = s.substr(1);
  s = StringUtil::toupper(s);
  std::vector<std::string> s_v = StringUtil::split(s, ' ');
  if (s_v.size() > 1 || StringUtil::find(s, "SW1") || StringUtil::find(s, "SW0")) {
    m_type = SET;
  } else {
    m_type = GET;
  }
  if (m_type == GET) {
    s_v = StringUtil::split(s, '=');
    if (StringUtil::find(s_v[0], "GET")) {
      m_command = ALL;
      if (s_v.size() > 1) {
        std::vector<std::string> ss_v = StringUtil::split(s_v[1], ',');
        setVoltageMon(decode(ss_v[0]));
        setCurrentMon(decode(ss_v[1]));
        setStatus((StatusFlag)atoi(ss_v[2].c_str()));
        setSwitchOn(ss_v[2] == "1");
        setRampUpSpeed(decode(ss_v[3]));
        setRampDownSpeed(decode(ss_v[4]));
      }
    } else if (StringUtil::find(s_v[0], "SW")) {
      m_command = SWITCH;
      if (s_v.size() > 1) {
        setSwitchOn(s_v[1] != "0");
        setStatus((StatusFlag)atoi(s_v[1].c_str()));
      }
    } else if (StringUtil::find(s_v[0], "RVU")) {
      m_command = RAMPUP_SPEED;
      if (s_v.size() > 1) setRampUpSpeed(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "RVD")) {
      m_command = RAMPDOWN_SPEED;
      if (s_v.size() > 1) setRampDownSpeed(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "CH5")) {
      m_command = VOLTAGE_DEMAND;
      if (s_v.size() > 1) setVoltageDemand(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "CH2")) {
      m_command = VOLTAGE_LIMIT;
      if (s_v.size() > 1) setVoltageLimit(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "CH7")) {
      m_command = CURRENT_LIMIT;
      if (s_v.size() > 1) setCurrentLimit(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "MNH1")) {
      m_command = VOLTAGE_MON;
      if (s_v.size() > 1) setVoltageMon(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "MNH2")) {
      m_command = CURRENT_MON;
      if (s_v.size() > 1) setCurrentMon(decode(s_v[1]));
    } else if (StringUtil::find(s_v[0], "DATE")) {
      m_command = DATE;
    } else if (StringUtil::find(s_v[0], "RECALL")) {
      m_command = RECALL;
      if (s_v.size() > 1) setStoreId(atoi(s_v[1].c_str()));
    } else if (StringUtil::find(s_v[0], "STORE")) {
      m_command = STORE;
      if (s_v.size() > 1) setStoreId(atoi(s_v[1].c_str()));
    }
  } else if (m_type == SET) {
    if (StringUtil::find(s_v[0], "SW0")) {
      m_command = SWITCH;
      setSwitchOn(false);
    } else if (StringUtil::find(s_v[0], "SW1")) {
      m_command = SWITCH;
      setSwitchOn(true);
    } else if (s_v.size() > 1) {
      std::string s = s_v[1];
      if (StringUtil::find(s_v[0], "SET")) {
        m_command = ALL;
        std::vector<std::string> ss_v = StringUtil::split(s, ',');
        if (ss_v.size() > 5) {
          setVoltageDemand(decode(ss_v[0]));
          setSwitchOn(ss_v[1] == "1");
          setStatus((StatusFlag)atoi(ss_v[2].c_str()));
          setVoltageLimit(decode(ss_v[2]));
          setCurrentLimit(decode(ss_v[3]));
          setRampUpSpeed(decode(ss_v[4]));
          setRampDownSpeed(decode(ss_v[5]));
        }
      } else if (StringUtil::find(s_v[0], "RVU")) {
        m_command = RAMPUP_SPEED;
        setRampUpSpeed(decode(s));
      } else if (StringUtil::find(s_v[0], "RVD")) {
        m_command = RAMPDOWN_SPEED;
        setRampDownSpeed(decode(s));
      } else if (StringUtil::find(s_v[0], "CH5")) {
        m_command = VOLTAGE_DEMAND;
        setVoltageDemand(decode(s));
      } else if (StringUtil::find(s_v[0], "CH2")) {
        m_command = VOLTAGE_LIMIT;
        setVoltageLimit(decode(s));
      } else if (StringUtil::find(s_v[0], "CH7")) {
        m_command = CURRENT_LIMIT;
        setCurrentLimit(decode(s));
      } else if (StringUtil::find(s_v[0], "RECALL")) {
        m_command = RECALL;
        setStoreId(atoi(s.c_str()));
      } else if (StringUtil::find(s_v[0], "STORE")) {
        m_command = STORE;
        setStoreId(atoi(s.c_str()));
      } else if (StringUtil::find(s_v[0], "DATE")) {
        m_command = DATE;
      }
    }
  }
}

