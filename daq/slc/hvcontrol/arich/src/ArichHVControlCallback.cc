#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"
#include <daq/slc/hvcontrol/arich/ArichHVCommunicator.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <iostream>
#include <list>

using namespace Belle2;

void ArichHVControlCallback::initialize(const HVConfig& hvconf) throw()
{
  const DBObject& obj(hvconf.get());
  const DBObjectList& c_crate_v(obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    const DBObject& c_crate(c_crate_v[i]);
    const std::string host = c_crate.getText("host");
    int crateid = i + 1;
    const int port = c_crate.getInt("port");
    const int usech  = c_crate.getInt("usech");
    ArichHVCommunicator acomm(crateid, host, port, usech, false);
    const DBObjectList& c_channel_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_channel_v.size(); j++) {
      const DBObject& c_channel(c_channel_v[j]);
      float voffset = c_channel.getFloat("voffset");
      float vslope = c_channel.getFloat("vslope");
      float coffset = c_channel.getFloat("coffset");
      float cslope = c_channel.getFloat("cslope");
      if (vslope <= 0) vslope = 1;
      if (cslope <= 0) cslope = 1;
      int slot = c_channel.getInt("slot");
      int channel = c_channel.getInt("channel");
      acomm.addUnit(ArichHVUnit(ArichHVCalib(voffset, vslope, coffset, cslope),
                                HVChannel(crateid, slot, channel)));
    }
    m_acomm.push_back(acomm);
  }
  for (size_t i = 0; i < m_acomm.size(); i++) {
    m_acomm[i].connect();
    PThread(new Listener(m_acomm[i]));
  }
}

void ArichHVControlCallback::update() throw()
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    m_acomm[i].requestValueAll(0, 0);
    if (getDB()) {
      DBInterface& db(*getDB());
      db.connect();
      for (int j = 0; j < m_acomm[i].getNUnits(); j++) {
        ArichHVUnit& unit(m_acomm[i].getUnit(j));
        HVValue& param(unit.getValue());
        HVChannel& ch(unit.getChannel());
        db.execute("insert into hvinfo (record_time, crate, "
                   "slot, channel, switchon, state, vmon, cmon) "
                   "values (current_timestamp, %d, %d, %d, %d, %d, %.1f, %.1f);",
                   m_acomm[i].getId(), ch.getSlot(),
                   ch.getChannel(), param.isTurnOn(),
                   unit.getStatus().getState(),
                   unit.getStatus().getVoltageMon(),
                   unit.getStatus().getCurrentMon());
      }
    }
  }
}

void ArichHVControlCallback::store(int index) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    m_acomm[i].store(index);
    m_acomm[i].requestValueAll(0, 0);
  }
}

void ArichHVControlCallback::recall(int index) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    m_acomm[i].recall(index);
    m_acomm[i].requestValueAll(0, 0);
  }
}

void ArichHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      if (switchon) m_acomm[i].switchOn(slot, channel);
      else m_acomm[i].switchOff(slot, channel);
      m_acomm[i].requestSwitch(slot, channel);
    }
  }
}

void ArichHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      m_acomm[i].setRampUpSpeed(slot, channel, rampup);
      m_acomm[i].requestRampUpSpeed(slot, channel);
    }
  }
}

void ArichHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      m_acomm[i].setRampDownSpeed(slot, channel, rampdown);
      m_acomm[i].requestRampDownSpeed(slot, channel);
    }
  }
}

void ArichHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      m_acomm[i].setVoltageDemand(slot, channel, voltage);
      m_acomm[i].requestVoltageDemand(slot, channel);
    }
  }
}

void ArichHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      m_acomm[i].setVoltageLimit(slot, channel, voltage);
      m_acomm[i].requestVoltageLimit(slot, channel);
    }
  }
}

void ArichHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == 0 || crate == m_acomm[i].getId()) {
      m_acomm[i].setCurrentLimit(slot, channel, current);
      m_acomm[i].requestCurrentLimit(slot, channel);
    }
  }
}

float ArichHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getRampUpSpeed(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getRampDownSpeed(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getVoltageDemand(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getVoltageLimit(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getCurrentLimit(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getVoltageMonitor(slot, channel);
  }
  return 0;
}

float ArichHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getCurrentMonitor(slot, channel);
  }
  return 0;
}

bool ArichHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getSwitch(slot, channel) > 0;
  }
  return false;
}

int ArichHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  for (size_t i = 0; i < m_acomm.size(); i++) {
    if (crate == m_acomm[i].getId()) return m_acomm[i].getState(slot, channel);
  }
  return 0;
}

void ArichHVControlCallback::Listener::run()
{
  while (true) {
    std::list<ArichHVMessage> msg_l(m_acomm.read());
    while (!msg_l.empty()) {
      ArichHVMessage msg = msg_l.front();
      msg_l.pop_front();
      if (msg.getMessageType() != ArichHVMessage::GET)
        continue;
      int unit = msg.getUnit();
      int channel = msg.getChannel();
      ArichHVUnitListIter it(m_acomm.find(unit, channel));
      if (it == m_acomm.getUnits().end()) continue;
      HVValue& value(it->getValue());
      HVStatus& status(it->getStatus());
      ArichHVCalib& calib(it->getCalib());
      switch (msg.getCommand()) {
        case ArichHVMessage::ALL: {
          status.setState((HVStatus::State)msg.getStatus());
          value.setTurnOn(msg.getSwitchOn());
          value.setRampUpSpeed(calib.decodeVoltage(msg.getRampUpSpeed()));
          value.setRampDownSpeed(calib.decodeVoltage(msg.getRampDownSpeed()));
          value.setVoltageDemand(calib.decodeVoltage(msg.getVoltageDemand()));
          value.setVoltageLimit(calib.decodeVoltage(msg.getVoltageLimit()));
          value.setCurrentLimit(calib.decodeCurrent(msg.getCurrentLimit()));
        } break;
        case ArichHVMessage::SWITCH: {
          status.setState((HVStatus::State)msg.getStatus());
          value.setTurnOn(msg.getSwitchOn());
        } break;
        case ArichHVMessage::RAMPUP_SPEED: {
          value.setRampUpSpeed(calib.decodeVoltage(msg.getRampUpSpeed()));
        } break;
        case ArichHVMessage::RAMPDOWN_SPEED: {
          value.setRampDownSpeed(calib.decodeVoltage(msg.getRampDownSpeed()));
        } break;
        case ArichHVMessage::VOLTAGE_DEMAND: {
          value.setVoltageDemand(calib.decodeVoltage(msg.getVoltageDemand()));
        } break;
        case ArichHVMessage::VOLTAGE_LIMIT: {
          value.setVoltageLimit(calib.decodeVoltage(msg.getVoltageLimit()));
        } break;
        case ArichHVMessage::CURRENT_LIMIT: {
          value.setCurrentLimit(calib.decodeCurrent(msg.getCurrentLimit()));
        } break;
        case ArichHVMessage::MON: {
          status.setState((HVStatus::State)msg.getStatus());
          status.setVoltageMon(calib.decodeVoltage(msg.getVoltageMon()));
          status.setCurrentMon(calib.decodeCurrent(msg.getCurrentMon()));
        } break;
        case ArichHVMessage::STATE: {
          status.setState((HVStatus::State)msg.getStatus());
        } break;
        case ArichHVMessage::VOLTAGE_MON: {
          status.setVoltageMon(calib.decodeVoltage(msg.getVoltageMon()));
        } break;
        case ArichHVMessage::CURRENT_MON: {
          status.setCurrentMon(calib.decodeCurrent(msg.getCurrentMon()));
        } break;
        default: break;
      }
    }
  }
}
