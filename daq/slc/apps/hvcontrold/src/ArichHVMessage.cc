#include "daq/slc/apps/hvcontrold/ArichHVMessage.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <iostream>
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
      case VOLTAGE_DEMAND: ss << "CH5?"; break;
      case VOLTAGE_LIMIT:  ss << "CH2?"; break;
      case CURRENT_LIMIT:  ss << "CH7?"; break;
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
      case VOLTAGE_DEMAND: ss << Belle2::form("CH5 %04X", m_info->getVoltageDemand()); break;
      case VOLTAGE_LIMIT:  ss << Belle2::form("CH2 %04X", m_info->getVoltageLimit()); break;
      case CURRENT_LIMIT:  ss << Belle2::form("CH7 %04X", m_info->getCurrentLimit()); break;
      default: break;
    }
  }
  return ss.str();
}

void ArichHVMessage::read(const std::string& str)
{
  std::string s = str;
  if (str.at(0) == '#') {
    m_info->setSlot(str.at(1) - '0');
    m_info->setChannel(str.at(2) - '0');
    s = s.substr(s.find(" ") + 1);
  } else {
    m_info->setSlot(0);
    m_info->setChannel(0);
  }
  s = Belle2::toupper(s);
  std::vector<std::string> s_v = Belle2::split(s, ' ');
  if (s_v.size() > 1 || s.find("SW") != std::string::npos) {
    m_command = SET;
  } else {
    m_command = GET;
  }
  if (m_command == GET) {
    s_v = Belle2::split(s, '=');
    if (s_v[0].find("GET") != std::string::npos) {
      m_type = ALL;
      if (s_v.size() > 1) {
        std::vector<std::string> ss_v = Belle2::split(s_v[1], ',');
        m_status->setVoltageMonitored(strtoul(("0x" + ss_v[0]).c_str(), 0, 0));
        m_status->setCurrentMonitored(strtoul(("0x" + ss_v[1]).c_str(), 0, 0));
        m_info->setSwitchOn(ss_v[2] == "1");
        m_status->setStatus(atoi(ss_v[2].c_str()));
        m_info->setRampUpSpeed(strtoul(("0x" + ss_v[3]).c_str(), 0, 0));
        m_info->setRampDownSpeed(strtoul(("0x" + ss_v[4]).c_str(), 0, 0));
      }
    } else if (s_v[0].find("SW") != std::string::npos) {
      m_type = SWITCH;
      if (s_v.size() > 1) {
        m_info->setSwitchOn(s_v[1] != "0");
        m_status->setStatus(atoi(s_v[1].c_str()));
      }
    } else if (s_v[0].find("RVU") != std::string::npos) {
      m_type = RAMPUP_SPEED;
      if (s_v.size() > 1) m_info->setRampUpSpeed(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("RVD") != std::string::npos) {
      m_type = RAMPDOWN_SPEED;
      if (s_v.size() > 1) m_info->setRampDownSpeed(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH5") != std::string::npos) {
      m_type = VOLTAGE_DEMAND;
      if (s_v.size() > 1) m_info->setVoltageDemand(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH2") != std::string::npos) {
      m_type = VOLTAGE_LIMIT;
      if (s_v.size() > 1) m_info->setVoltageLimit(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("CH7") != std::string::npos) {
      m_type = CURRENT_LIMIT;
      if (s_v.size() > 1) m_info->setCurrentLimit(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("MNH1") != std::string::npos) {
      m_type = VOLTAGE_MON;
      if (s_v.size() > 1) m_status->setVoltageMonitored(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("MNH2") != std::string::npos) {
      m_type = CURRENT_MON;
      if (s_v.size() > 1) m_status->setCurrentMonitored(strtoul(("0x" + s_v[1]).c_str(), 0, 0));
    } else if (s_v[0].find("DATE") != std::string::npos) {
      m_type = DATE;
    }
  } else if (m_command == SET) {
    if (s_v[0] == "SW0") {
      m_type = SWITCH;
      m_info->setSwitchOn(false);
    } else if (s_v[0] == "SW1") {
      m_type = SWITCH;
      m_info->setSwitchOn(true);
    } else if (s_v.size() > 1) {
      std::string s = s_v[1];
      if (s_v[0] == "SET") {
        m_type = ALL;
        if (s_v.size() > 5) {
          std::vector<std::string> ss_v = Belle2::split(s, ',');
          m_info->setVoltageDemand(strtoul(("0x" + ss_v[0]).c_str(), 0, 0));
          m_info->setSwitchOn(ss_v[1] == "1");
          m_info->setVoltageLimit(strtoul(("0x" + ss_v[2]).c_str(), 0, 0));
          m_info->setCurrentLimit(strtoul(("0x" + ss_v[3]).c_str(), 0, 0));
          m_info->setRampUpSpeed(strtoul(("0x" + ss_v[4]).c_str(), 0, 0));
          m_info->setRampDownSpeed(strtoul(("0x" + ss_v[5]).c_str(), 0, 0));
        }
      } else if (s_v[0] == "RVU") {
        m_type = RAMPUP_SPEED;
        m_info->setRampUpSpeed(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0] == "RVD") {
        m_type = RAMPDOWN_SPEED;
        m_info->setRampDownSpeed(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0] == "CH5") {
        m_type = VOLTAGE_DEMAND;
        m_info->setVoltageDemand(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0] == "CH2") {
        m_type = VOLTAGE_LIMIT;
        m_info->setVoltageLimit(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0] == "CH7") {
        m_type = CURRENT_LIMIT;
        m_info->setCurrentLimit(strtoul(("0x" + s).c_str(), 0, 0));
      } else if (s_v[0] == "DATE") {
        m_type = DATE;
      }
    }
  }
}
