#include "daq/slc/hvcontrol/arich/ArichHVCommunicator.h"

#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>

using namespace Belle2;

Mutex g_mutex;

ArichHVCommunicator::ArichHVCommunicator(int crateid,
                                         const std::string& host,
                                         int port, int use_ch, int debug)
  : m_crateid(crateid), m_socket(host, port),
    m_available(false), m_usech(use_ch), m_debug(debug)
{
}

ArichHVUnitListIter ArichHVCommunicator::find(int unit, int channel)
{
  for (ArichHVUnitListIter it = m_unit.begin();
       it != m_unit.end(); it++) {
    HVChannel& ch(it->getChannel());
    if ((unit == 0 || ch.getSlot() == unit) &&
        (channel == 0 || ch.getChannel() == channel)) {
      return it;
    }
  }
  return m_unit.end();
}

void ArichHVCommunicator::connect() throw(IOException)
{
  m_mutex.lock();
  m_available = false;
  try {
    m_socket.connect();
    m_writer = TCPSocketWriter(m_socket);
    m_reader = TCPSocketReader(m_socket);
    m_available = true;
  } catch (const IOException& e) {
    m_socket.close();
    m_available = false;
    m_mutex.unlock();
    LogFile::debug("Socket connection error to HV crate: %s", e.what());
    throw (e);
  }
  m_mutex.unlock();
}

void ArichHVCommunicator::store(int index) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::STORE, 0);
  msg.setStoreId(index);
  send(msg);
}

void ArichHVCommunicator::recall(int index) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0);
  msg.setStoreId(index);
  send(msg);
}

void ArichHVCommunicator::setRampUpSpeed(int unit, int ch, float rampup) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return;
  it->getValue().setRampUpSpeed(rampup);
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RAMPUP_SPEED, unit, ch);
  msg.setRampUpSpeed(it->getCalib().encodeVoltage(rampup));
  send(msg);
}

void ArichHVCommunicator::setRampDownSpeed(int unit, int ch, float rampdown) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return;
  it->getValue().setRampDownSpeed(rampdown);
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RAMPDOWN_SPEED, unit, ch);
  msg.setRampDownSpeed(it->getCalib().encodeVoltage(rampdown));
  send(msg);
}

void ArichHVCommunicator::setVoltageDemand(int unit, int ch, float voltage) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return;
  it->getValue().setVoltageDemand(voltage);
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::VOLTAGE_DEMAND, unit, ch);
  msg.setVoltageDemand(it->getCalib().encodeVoltage(voltage));
  send(msg);
}

void ArichHVCommunicator::setVoltageLimit(int unit, int ch, float voltage) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return;
  it->getValue().setVoltageLimit(voltage);
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::VOLTAGE_LIMIT, unit, ch);
  msg.setVoltageLimit(it->getCalib().encodeVoltage(voltage));
  send(msg);
}

void ArichHVCommunicator::setCurrentLimit(int unit, int ch, float current) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return;
  it->getValue().setCurrentLimit(current);
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::CURRENT_LIMIT, unit, ch);
  msg.setCurrentLimit(it->getCalib().encodeCurrent(current));
  send(msg);
}

float ArichHVCommunicator::getRampUpSpeed(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().getRampUpSpeed();
}

float ArichHVCommunicator::getRampDownSpeed(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().getRampDownSpeed();
}

float ArichHVCommunicator::getVoltageDemand(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().getVoltageDemand();
}

float ArichHVCommunicator::getVoltageLimit(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().getVoltageLimit();
}

float ArichHVCommunicator::getCurrentLimit(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().getCurrentLimit();
}

int ArichHVCommunicator::getState(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getStatus().getState();
}

float ArichHVCommunicator::getVoltageMonitor(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getStatus().getVoltageMon();
}

float ArichHVCommunicator::getCurrentMonitor(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getStatus().getCurrentMon();
}

void ArichHVCommunicator::switchOn(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (!(unit == 0 && ch == 0) && it == m_unit.end()) return;
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::SWITCH, unit, ch);
  msg.setSwitchOn(true);
  send(msg);
  it->getValue().setTurnOn(true);
}

void ArichHVCommunicator::switchOff(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (!(unit == 0 && ch == 0) && it == m_unit.end()) return;
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::SWITCH, unit, ch);
  msg.setSwitchOn(false);
  send(msg);
  it->getValue().setTurnOn(false);
}

int ArichHVCommunicator::getSwitch(int unit, int ch) throw(IOException)
{
  ArichHVUnitListIter it = find(unit, ch);
  if (it == m_unit.end()) return 0;
  return it->getValue().isTurnOn();
}

void ArichHVCommunicator::requestValueAll(int unit, int ch) throw(IOException)
{
  requestSwitch(unit, ch);
  requestRampUpSpeed(unit, ch);
  requestRampDownSpeed(unit, ch);
  requestVoltageDemand(unit, ch);
  requestVoltageLimit(unit, ch);
  requestCurrentLimit(unit, ch);
  requestVoltageMonitor(unit, ch);
  requestCurrentMonitor(unit, ch);
}

void ArichHVCommunicator::requestSwitch(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::SWITCH, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestRampUpSpeed(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::RAMPUP_SPEED, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestRampDownSpeed(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::RAMPDOWN_SPEED, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestVoltageDemand(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::VOLTAGE_DEMAND, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestVoltageLimit(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::VOLTAGE_LIMIT, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestCurrentLimit(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::CURRENT_LIMIT, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestVoltageMonitor(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::VOLTAGE_MON, unit, ch);
  send(msg);
}

void ArichHVCommunicator::requestCurrentMonitor(int unit, int ch) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::CURRENT_MON, unit, ch);
  send(msg);
}

void ArichHVCommunicator::send(ArichHVMessage& msg)
throw(IOException)
{
  if (!m_available) {
    LogFile::error("HV crate is Not available");
    return;
  }
  m_mutex.lock();
  try {
    std::string str = msg.toString(m_usech);
    if (m_debug) {
      std::cerr << str << std::endl;
    }
    str += "\r";
    m_writer.write(str.c_str(), str.size());
    m_mutex.unlock();
  } catch (const IOException& e) {
    LogFile::debug("Socket error on HV crate: %s", e.what());
    m_socket.close();
    m_available = false;
    m_mutex.unlock();
    throw (e);
  }
}

std::list<ArichHVMessage> ArichHVCommunicator::read()
throw(IOException)
{
  std::stringstream ss;
  char c;
  std::vector<std::string> s;
  while (true) {
    c = m_reader.readChar();
    if (c == '\r') break;
    if (c == '#') {
      s.push_back(ss.str());
      ss.str("");
    }
    ss << c;
  }
  if (ss.str().size() > 0)
    s.push_back(ss.str());
  std::list<ArichHVMessage> msg_l;
  for (size_t i = 0; i < s.size(); i++) {
    if (useChannel()) {
      ArichHVMessage msg;
      StringList sl = StringUtil::split(s[i], '=');
      if (sl.size() >= 2) {
        StringList sll = StringUtil::split(sl[1], ',');
        for (size_t j = 0; j < sll.size(); j++) {
          if (sll.size() > 0) {
            std::stringstream css;
            css << StringUtil::form("#%c%d", sl[0].at(1), (int)j + 1)
                << sl[0].substr(3) << "=" << sll[j];
            msg.read(useChannel(), css.str());
            msg_l.push_back(msg);
          }
        }
      }
    } else {
      if (s[i].size() > 0) {
        s[i] = StringUtil::replace(s[i], ",", "");
        ArichHVMessage msg;
        msg.read(useChannel(), s[i]);
        msg_l.push_back(msg);
      }
    }
  }
  return msg_l;
}
