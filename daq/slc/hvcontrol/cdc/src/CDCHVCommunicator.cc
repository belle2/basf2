#include "daq/slc/hvcontrol/cdc/CDCHVCommunicator.h"

#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/system/LogFile.h>

#include <sstream>

using namespace Belle2;

CDCHVCommunicator::CDCHVCommunicator(HVControlCallback* callback,
                                     unsigned int crateid,
                                     const std::string& host, int port)
  : m_callback(callback), m_crateid(crateid),
    m_socket(host, port), m_available(false)
{
}

bool CDCHVCommunicator::connect() throw()
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
    return false;
  }
  m_mutex.unlock();
  return true;
}

bool CDCHVCommunicator::configure() throw()
{
  bool success = false;
  m_mutex.lock();
  if (m_available) {
    try {
      {
        CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::SWITCH, 0);
        msg.setSwitchOn(false);
        send(msg);
      }
      const HVConfig& config(m_callback->getConfig());
      for (size_t j = 0; j < config.getNValueSets(); j++) {
        const HVValueSet& value_v(config.getValueSet(j));
        for (size_t i = 0; i < m_callback->getNChannels(); i++) {
          const HVChannel& channel(config.getChannel(i));
          if (channel.getCrate() != m_crateid) continue;
          const HVValue& value(value_v[i]);
          CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::ALL,
                           /*channel.getSlot(), */channel.getChannel());
          msg.setVoltageLimit(value.getVoltageLimit());
          msg.setCommand(CDCHVMessage::VOLTAGE_LIMIT);
          send(msg);
          msg.setCurrentLimit(value.getCurrentLimit());
          msg.setCommand(CDCHVMessage::CURRENT_LIMIT);
          send(msg);
          msg.setRampUpSpeed(value.getRampUpSpeed());
          msg.setCommand(CDCHVMessage::RAMPUP_SPEED);
          send(msg);
          msg.setRampDownSpeed(value.getRampDownSpeed());
          msg.setCommand(CDCHVMessage::RAMPDOWN_SPEED);
          send(msg);
          msg.setCommand(CDCHVMessage::VOLTAGE_DEMAND);
          int voltage_demand = value.getVoltageDemand() * channel.isTurnOn();
          msg.setVoltageDemand(voltage_demand);
          send(msg);
        }
        CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::STORE, 0);
        msg.setStoreId(j + 1);
        send(msg);
      }
      success = true;
    } catch (const IOException& e) {
      LogFile::error("failed to configure HV crate (id=%s)", m_crateid);
      success = false;
    }
  }
  m_mutex.unlock();
  return success;
}

bool CDCHVCommunicator::turnon() throw()
{
  m_mutex.lock();
  CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::RECALL, 0);
  msg.setStoreId(1);
  send(msg);
  m_mutex.unlock();
  msg.setCommand(CDCHVMessage::SWITCH);
  msg.setSwitchOn(true);
  return perform(msg, HVState::STANDBY_S);
}

bool CDCHVCommunicator::turnoff() throw()
{
  CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::SWITCH, 0);
  msg.setSwitchOn(false);
  return perform(msg, HVState::OFF_S);
}

bool CDCHVCommunicator::standby() throw()
{
  CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::RECALL, 0);
  msg.setStoreId(1);
  return perform(msg, HVState::STANDBY_S);
}

bool CDCHVCommunicator::shoulder() throw()
{
  CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::RECALL, 0);
  msg.setStoreId(2);
  return perform(msg, HVState::SHOULDER_S);
}

bool CDCHVCommunicator::peak() throw()
{
  CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::RECALL, 0);
  msg.setStoreId(4);
  return perform(msg, HVState::PEAK_S);
}

CDCHVMessage CDCHVCommunicator::readParams(/*int slot, */int channel) throw(IOException)
{
  CDCHVMessage msg(CDCHVMessage::GET, CDCHVMessage::ALL, /*slot, */channel);
  msg.read(send(msg));
  msg.setCommand(CDCHVMessage::CURRENT_LIMIT);
  msg.read(send(msg));
  msg.setCommand(CDCHVMessage::VOLTAGE_LIMIT);
  msg.read(send(msg));
  msg.setCommand(CDCHVMessage::VOLTAGE_DEMAND);
  msg.read(send(msg));
  return msg;
}

std::string CDCHVCommunicator::send(CDCHVMessage& msg)
throw(IOException)
{
  std::string s;
  try {
    std::string str = msg.toString() + "\r";
    m_writer.write(str.c_str(), str.size());
    if (msg.getMessageType() == CDCHVMessage::GET) {
      std::stringstream ss;
      char c;
      while (true) {
        m_socket.select(10);
        c = m_reader.readChar();
        if (c == '\r') break;
        ss << c;
      }
      s = ss.str();
    }
  } catch (const IOException& e) {
    LogFile::debug("Socket error on HV crate: %s", e.what());
    m_socket.close();
    m_available = false;
    throw (e);
  }
  return s;
}

bool CDCHVCommunicator::perform(CDCHVMessage& msg, HVState state) throw()
{
  bool success = false;
  m_mutex.lock();
  if (m_available) {
    try {
      send(msg);
      const HVConfig& config(m_callback->getConfig());
      for (size_t i = 0; i < m_callback->getNChannels(); i++) {
        const HVChannel& channel(config.getChannel(i));
        if (channel.getCrate() != m_crateid && channel.isTurnOn()) {
          HVChannelStatus& status(m_callback->getChannelStatus(i));
          HVState state_org = status.getState();
          if (state_org.isStable()) {
            if (state.getId() > state_org.getId()) {
              status.setState(HVState::RAMPINGUP_TS);
            } else if (state.getId() < state_org.getId()) {
              status.setState(HVState::RAMPINGDOWN_TS);
            }
          } else {
            status.setState(HVState::TRANSITION_TS);
          }
        }
      }
      success = true;
    } catch (const IOException& e) {
      LogFile::error("failed to configure HV crate (id=%s)", m_crateid);
      success = false;
    }
  }
  m_mutex.unlock();
  return success;
}

