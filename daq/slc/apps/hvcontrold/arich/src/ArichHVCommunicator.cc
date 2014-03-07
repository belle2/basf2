#include "daq/slc/apps/hvcontrold/arich/ArichHVCommunicator.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include <daq/slc/system/LogFile.h>

#include <sstream>

using namespace Belle2;

ArichHVCommunicator::ArichHVCommunicator(HVControlCallback* callback,
                                         unsigned int crateid,
                                         const std::string& host, int port)
  : m_callback(callback), m_crateid(crateid),
    m_socket(host, port), m_available(false)
{
}

bool ArichHVCommunicator::connect() throw()
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

bool ArichHVCommunicator::configure() throw()
{
  bool success = false;
  m_mutex.lock();
  if (m_available) {
    try {
      {
        ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::SWITCH, 0, 0);
        msg.setSwitchOn(false);
        request(msg);
      }
      HVNodeInfo& info(m_callback->getInfo());
      for (HVChannelInfoList::iterator it = info.getChannelList().begin();
           it != info.getChannelList().end(); it++) {
        HVChannelConfig& config(it->getConfig());
        if (config.getCrate() != m_crateid) continue;
        ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::ALL,
                           config.getSlot(), config.getChannel());
        msg.setVoltageLimit(config.getVoltageLimit());
        msg.setCommand(ArichHVMessage::VOLTAGE_LIMIT);
        request(msg);
        msg.setCurrentLimit(config.getCurrentLimit());
        msg.setCommand(ArichHVMessage::CURRENT_LIMIT);
        request(msg);
        msg.setRampUpSpeed(config.getRampUpSpeed());
        msg.setCommand(ArichHVMessage::RAMPUP_SPEED);
        request(msg);
        msg.setRampDownSpeed(config.getRampDownSpeed());
        msg.setCommand(ArichHVMessage::RAMPDOWN_SPEED);
        request(msg);
      }
      for (int i = 0; i < 4; i++) {
        for (HVChannelInfoList::iterator it = info.getChannelList().begin();
             it != info.getChannelList().end(); it++) {
          HVChannelConfig& config(it->getConfig());
          if (config.getCrate() != m_crateid) continue;
          ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::VOLTAGE_DEMAND,
                             config.getSlot(), config.getChannel());
          int voltage_demand = config.getVoltageDemand((HVDemand)i) * config.isTurnOn();
          msg.setVoltageDemand(voltage_demand);
          request(msg);
        }
        ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::STORE, 0, 0);
        msg.setStoreId(i + 1);
        request(msg);
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

bool ArichHVCommunicator::turnon() throw()
{
  m_mutex.lock();
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0, 0);
  msg.setStoreId((int)HVDemand::STANDBY + 1);
  request(msg);
  m_mutex.unlock();
  msg.setCommand(ArichHVMessage::SWITCH);
  msg.setSwitchOn(true);
  return perform(msg, HVState::STANDBY_S);
}

bool ArichHVCommunicator::turnoff() throw()
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::SWITCH, 0, 0);
  msg.setSwitchOn(false);
  return perform(msg, HVState::OFF_S);
}

bool ArichHVCommunicator::standby() throw()
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0, 0);
  msg.setStoreId((int)HVDemand::STANDBY + 1);
  return perform(msg, HVState::STANDBY_S);
}

bool ArichHVCommunicator::standby2() throw()
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0, 0);
  msg.setStoreId((int)HVDemand::STANDBY2 + 1);
  return perform(msg, HVState::STANDBY2_S);
}

bool ArichHVCommunicator::standby3() throw()
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0, 0);
  msg.setStoreId((int)HVDemand::STANDBY3 + 1);
  return perform(msg, HVState::STANDBY3_S);
}

bool ArichHVCommunicator::peak() throw()
{
  ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::RECALL, 0, 0);
  msg.setStoreId((int)HVDemand::PEAK + 1);
  return perform(msg, HVState::PEAK_S);
}

ArichHVMessage ArichHVCommunicator::readParams(int slot, int channel) throw(IOException)
{
  ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::ALL, slot, channel);
  msg.read(request(msg));
  msg.setCommand(ArichHVMessage::CURRENT_LIMIT);
  msg.read(request(msg));
  msg.setCommand(ArichHVMessage::VOLTAGE_LIMIT);
  msg.read(request(msg));
  msg.setCommand(ArichHVMessage::VOLTAGE_DEMAND);
  msg.read(request(msg));
  return msg;
}

std::string ArichHVCommunicator::request(ArichHVMessage& msg)
throw(IOException)
{
  std::string s;
  try {
    std::string str = msg.toString() + "\r";
    //LogFile::debug(str);
    m_writer.write(str.c_str(), str.size());
    if (msg.getMessageType() == ArichHVMessage::GET) {
      std::stringstream ss;
      char c;
      while (true) {
        m_socket.select(10);
        c = m_reader.readChar();
        if (c == '\r') break;
        ss << c;
      }
      s = ss.str();
      //LogFile::debug(s);
    }
  } catch (const IOException& e) {
    LogFile::debug("Socket error on HV crate: %s", e.what());
    m_socket.close();
    m_available = false;
    throw (e);
  }
  return s;
}

bool ArichHVCommunicator::perform(ArichHVMessage& msg, HVState state) throw()
{
  bool success = false;
  m_mutex.lock();
  if (m_available) {
    try {
      request(msg);
      HVNodeInfo& info(m_callback->getInfo());
      for (HVChannelInfoList::iterator it = info.getChannelList().begin();
           it != info.getChannelList().end(); it++) {
        HVChannelConfig& config(it->getConfig());
        if (config.getCrate() == m_crateid && config.isTurnOn()) {
          HVChannelStatus& status(it->getStatus());
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

