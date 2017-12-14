#include "daq/slc/hvcontrol/arich/ArichHVTUI.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <daq/slc/system/PThread.h>

#include <unistd.h>
#include <ncurses.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

using namespace Belle2;

ArichHVTUI::ArichHVTUI()
{

}

void ArichHVTUI::initialize(const std::string& confname,
                            const std::string& calibname)
{
  ConfigFile cfile(confname, calibname);
  m_crateid = cfile.getInt("hv.crateid");
  const std::string host = cfile.get("hv.hostname");
  const int port = cfile.getInt("hv.port");
  const int max_unit  = cfile.getInt("hv.nchannels");
  const int mode = cfile.getInt("hv.digitmode");
  const int usech = cfile.getInt("hv.usechannel");
  const int crateid = cfile.getInt("hv.crateid");
  m_debug = cfile.getInt("hv.debug");
  m_comm = ArichHVCommunicator(m_crateid, host, port, usech, m_debug);
  for (int i = 0; i < max_unit; i++) {
    float voffset = cfile.getFloat(StringUtil::form("hv.channel[%d].voffset", i + 1));
    float vslope = cfile.getFloat(StringUtil::form("hv.channel[%d].vslope", i + 1));
    float coffset = cfile.getFloat(StringUtil::form("hv.channel[%d].coffset", i + 1));
    float cslope = cfile.getFloat(StringUtil::form("hv.channel[%d].cslope", i + 1));
    if (vslope <= 0) vslope = 1;
    if (cslope <= 0) cslope = 1;
    if (mode != 0) {
      voffset = coffset = 0;
      vslope = cslope = 1;
    }
    int slot = cfile.getInt(StringUtil::form("hv.channel[%d].slot", i + 1));
    int channel = cfile.getInt(StringUtil::form("hv.channel[%d].channel", i + 1));
    m_comm.addUnit(ArichHVUnit(ArichHVCalib(voffset, vslope, coffset, cslope),
                               HVChannel(crateid, slot, channel)));
  }
  m_comm.connect();
  init(mode, cfile.getInt("hv.interval"));
}

ArichHVTUI::~ArichHVTUI() throw()
{
}

void ArichHVTUI::store(int index) throw(IOException)
{
  m_comm.store(index);
  getValueAll();
}

void ArichHVTUI::recall(int index) throw(IOException)
{
  m_comm.recall(index);
  getValueAll();
}

void ArichHVTUI::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  m_comm.setRampUpSpeed(slot, channel, rampup);
  m_comm.requestRampUpSpeed(slot, channel);
}

void ArichHVTUI::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  m_comm.setRampDownSpeed(slot, channel, rampdown);
  m_comm.requestRampDownSpeed(slot, channel);
}

void ArichHVTUI::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_comm.setVoltageDemand(slot, channel, voltage);
  m_comm.requestVoltageDemand(slot, channel);
}

void ArichHVTUI::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  m_comm.setVoltageLimit(slot, channel, voltage);
  m_comm.requestVoltageLimit(slot, channel);
}

void ArichHVTUI::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  m_comm.setCurrentLimit(slot, channel, current);
  m_comm.requestCurrentLimit(slot, channel);
}

void ArichHVTUI::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  if (switchon) m_comm.switchOn(slot, channel);
  else m_comm.switchOff(slot, channel);
  if (slot == 0 && channel == 0) {
    for (int i = 0; i < m_comm.getNUnits(); i++) {
      HVChannel& ch(m_comm.getUnit(i).getChannel());
      m_comm.requestSwitch(ch.getSlot(), ch.getChannel());
    }
  } else {
    m_comm.requestSwitch(slot, channel);
  }
}

void ArichHVTUI::requestSwitch(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestSwitch(slot, channel);
}

void ArichHVTUI::requestRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestRampUpSpeed(slot, channel);
}

void ArichHVTUI::requestRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestRampDownSpeed(slot, channel);
}

void ArichHVTUI::requestVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestVoltageDemand(slot, channel);
}

void ArichHVTUI::requestVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestVoltageLimit(slot, channel);
}

void ArichHVTUI::requestCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestCurrentLimit(slot, channel);
}

void ArichHVTUI::requestVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestVoltageMonitor(slot, channel);
}

void ArichHVTUI::requestCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  m_comm.requestCurrentMonitor(slot, channel);
}

HVMessage ArichHVTUI::wait() throw(IOException)
{
  ArichHVMessage msg;
  while (true) {
    if (m_msg_l.empty()) m_msg_l = m_comm.read();
    msg = m_msg_l.front();
    m_msg_l.pop_front();
    if (msg.getMessageType() == ArichHVMessage::GET) {
      break;
    }
  }
  int slot = msg.getUnit();
  int channel = msg.getChannel();
  ArichHVMessage::Command command = msg.getCommand();
  ArichHVUnitListIter it(m_comm.find(slot, channel));
  if (it == m_comm.getUnits().end()) return HVMessage();
  ArichHVCalib& calib(it->getCalib());
  switch (command) {
    case ArichHVMessage::ALL: {
      return HVMessage(m_crateid, slot, channel, msg.getSwitchOn(),
                       calib.decodeVoltage(msg.getVoltageDemand()),
                       calib.decodeVoltage(msg.getVoltageLimit()),
                       calib.decodeCurrent(msg.getCurrentLimit()),
                       calib.decodeVoltage(msg.getRampUpSpeed()),
                       calib.decodeVoltage(msg.getRampDownSpeed()));
    } break;
    case ArichHVMessage::SWITCH:
      return HVMessage(m_crateid, slot, channel, HVMessage::SWITCH, msg.getSwitchOn());
    case ArichHVMessage::RAMPUP_SPEED:
      return HVMessage(m_crateid, slot, channel, HVMessage::RAMPUP_SPEED,
                       calib.decodeVoltage(msg.getRampUpSpeed()));
    case ArichHVMessage::RAMPDOWN_SPEED:
      return HVMessage(m_crateid, slot, channel, HVMessage::RAMPDOWN_SPEED,
                       calib.decodeVoltage(msg.getRampDownSpeed()));
    case ArichHVMessage::VOLTAGE_DEMAND:
      return HVMessage(m_crateid, slot, channel, HVMessage::VOLTAGE_DEMAND,
                       calib.decodeVoltage(msg.getVoltageDemand()));
    case ArichHVMessage::VOLTAGE_LIMIT:
      return HVMessage(m_crateid, slot, channel, HVMessage::VOLTAGE_LIMIT,
                       calib.decodeVoltage(msg.getVoltageLimit()));
    case ArichHVMessage::CURRENT_LIMIT:
      return HVMessage(m_crateid, slot, channel, HVMessage::CURRENT_LIMIT,
                       calib.decodeCurrent(msg.getCurrentLimit()));
    case ArichHVMessage::MON:
      return HVMessage(m_crateid, slot, channel, (int)msg.getStatus(),
                       calib.decodeVoltage(msg.getVoltageMon()),
                       calib.decodeVoltage(msg.getCurrentMon()));
    case ArichHVMessage::STATE:
      return HVMessage(m_crateid, slot, channel, HVMessage::STATE,
                       (int)msg.getStatus());
    case ArichHVMessage::VOLTAGE_MON:
      return HVMessage(m_crateid, slot, channel, HVMessage::VOLTAGE_MON,
                       calib.decodeVoltage(msg.getVoltageMon()));
    case ArichHVMessage::CURRENT_MON:
      return HVMessage(m_crateid, slot, channel, HVMessage::CURRENT_MON,
                       calib.decodeCurrent(msg.getCurrentMon()));
    default: break;
  }
  return HVMessage();
}
