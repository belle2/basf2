#include "ArichHVMessage.h"

#include "base/Debugger.h"
#include "base/StringUtil.h"

#include <sstream>
#include <cstdlib>

using namespace Belle2;

std::string ArichHVMessage::toString()
{
  std::stringstream ss;
  ss << "#" << m_info->getSlot() << m_info->getChannel() << " ";
  if (m_command == GET) {
    switch (m_type) {
      case ALL:            ss << "GET"; break;
      case SWITCH:         ss << "SW?"; break;
      case RAMPUP_SPEED:   ss << "RVU?"; break;
      case RAMPDOWN_SPEED: ss << "RVD?"; break;
      case VOLTAGE_DEMAND: ss << "CH2?"; break;
      case VOLTAGE_LIMIT:  ss << "CH7?"; break;
      case CURRENT_LIMIT:  ss << "CH5?"; break;
      case VOLTAGE_MON:    ss << "MNH1"; break;
      case CURRENT_MON:    ss << "MNH2"; break;
      case DATE:           ss << "DATE?"; break;
    }
  } else if (m_command == SET) {
    switch (m_type) {
      case ALL: ss << Belle2::form("SET %04X,%d,%04X,%04X,%04X", m_info->getVoltageDemand(),
                                     (int)m_info->isSwitchOn(), m_info->getVoltageLimit(),
                                     m_info->getCurrentLimit(), m_info->getRampUpSpeed(),
                                     m_info->getRampDownSpeed()); break;
      case SWITCH:         ss << "SW" << (int)m_info->isSwitchOn(); break;
      case RAMPUP_SPEED:   ss << Belle2::form("RVU %04X", m_info->getRampUpSpeed()); break;
      case RAMPDOWN_SPEED: ss << Belle2::form("RVD %04X", m_info->getRampDownSpeed()); break;
      case VOLTAGE_DEMAND: ss << Belle2::form("CH2 %04X", m_info->getVoltageDemand()); break;
      case VOLTAGE_LIMIT:  ss << Belle2::form("CH7 %04X", m_info->getVoltageLimit()); break;
      case CURRENT_LIMIT:  ss << Belle2::form("CH5 %04X", m_info->getCurrentLimit()); break;
      default: break;
    }
  }
  return ss.str();
}

void ArichHVMessage::read(const std::string& str)
{
  std::vector<std::string> s_v;
  if (str.at(0) == '#') {
    m_info->setSlot(str.at(1));
    m_info->setChannel(str.at(2));
    s_v = Belle2::split(str.substr(3), '=');
  } else {
    m_info->setSlot(0);
    m_info->setChannel(0);
    s_v = Belle2::split(str, '=');
  }
  s_v[0] = Belle2::toupper(s_v[0]);
  if (s_v[0] == "GET") {
    m_command = GET;
    m_type = ALL;
  } else if (s_v[0] == "SW?") {
    m_command = GET;
    m_type = SWITCH;
  } else if (s_v[0] == "RVU?") {
    m_command = GET;
    m_type = RAMPUP_SPEED;
  } else if (s_v[0] == "RVD?") {
    m_command = GET;
    m_type = RAMPDOWN_SPEED;
  } else if (s_v[0] == "CH2?") {
    m_command = GET;
    m_type = VOLTAGE_DEMAND;
  } else if (s_v[0] == "CH7?") {
    m_command = GET;
    m_type = VOLTAGE_LIMIT;
  } else if (s_v[0] == "CH5?") {
    m_command = GET;
    m_type = CURRENT_LIMIT;
  } else if (s_v[0] == "MNH1") {
    m_command = GET;
    m_type = VOLTAGE_MON;
  } else if (s_v[0] == "MNH2") {
    m_command = GET;
    m_type = CURRENT_MON;
  } else if (s_v[0] == "DATE?") {
    m_command = GET;
    m_type = DATE;
  }
  if (m_command == GET) {
    std::string s = s_v[1];
    switch (m_type) {
      case ALL: {
        std::vector<std::string> ss_v = Belle2::split(s, ',');
        m_info->setVoltageMonitored(strtoul(("0x" + ss_v[0]).c_str(), 0, 0));
        m_info->setCurrentMonitored(strtoul(("0x" + ss_v[1]).c_str(), 0, 0));
        m_info->setSwitchOn(ss_v[2] == "1");
        m_info->setRampUpSpeed(strtoul(("0x" + ss_v[3]).c_str(), 0, 0));
        m_info->setRampDownSpeed(strtoul(("0x" + ss_v[4]).c_str(), 0, 0));
      }; break;
      case SWITCH:
        m_info->setSwitchOn(s != "0"); break;
      case RAMPUP_SPEED:
        m_info->setRampUpSpeed(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case RAMPDOWN_SPEED:
        m_info->setRampDownSpeed(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case VOLTAGE_DEMAND:
        m_info->setVoltageDemand(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case VOLTAGE_LIMIT:
        m_info->setVoltageLimit(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case CURRENT_LIMIT:
        m_info->setCurrentLimit(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case VOLTAGE_MON:
        m_info->setVoltageMonitored(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case CURRENT_MON:
        m_info->setCurrentMonitored(strtoul(("0x" + s).c_str(), 0, 0)); break;
      case DATE: break;
    }
  }
}
