#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"

#include <daq/slc/hvcontrol/arich/ArichHVCommunicator.h>
#include <daq/slc/hvcontrol/arich/ArichHVMonitor.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <iostream>

using namespace Belle2;

ArichHVControlCallback::~ArichHVControlCallback() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    delete m_comm_v[i];
  }
}

void ArichHVControlCallback::initialize() throw()
{
  ConfigFile config("hvcontrol/arich");
  const int ncrates = config.getInt("arich.hv.ncrates");
  for (int n = 0; n < ncrates; n++) {
    const std::string host = config.get(StringUtil::form("arich.hv.%d.host", n));
    int port = config.getInt(StringUtil::form("arich.hv.%d.port", n));
    ArichHVCommunicator* comm = new ArichHVCommunicator(this, n, host, port);
    PThread(new ArichHVMonitor(comm));
    m_comm_v.push_back(comm);
  }
}

bool ArichHVControlCallback::configure() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    m_comm_v[i]->configure();
  }
  return true;
}

bool ArichHVControlCallback::turnon() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnon();
  }
  return success;
}

bool ArichHVControlCallback::turnoff() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnoff();
  }
  return success;
}

bool ArichHVControlCallback::standby() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->standby();
  }
  return success;
}

bool ArichHVControlCallback::shoulder() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->shoulder();
  }
  return success;
}

bool ArichHVControlCallback::peak() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->peak();
  }
  return success;
}

bool ArichHVControlCallback::hvapply(const HVApplyMessage& hvmsg)
throw()
{
  for (size_t i = 0; i < getNChannels(); i++) {
    const HVChannel& channel(getConfig().getChannel(i));
    if ((hvmsg.isAllCrates() || hvmsg.getCrate() == channel.getCrate()) &&
        (hvmsg.isAllSlots() || hvmsg.getSlot() == channel.getSlot()) &&
        (hvmsg.isAllChannels() || hvmsg.getChannel() == channel.getChannel())) {
      ArichHVMessage msg(ArichHVMessage::SET, ArichHVMessage::ALL,
                         channel.getSlot(), channel.getChannel());
      std::cout << i << " " << channel.getCrate() << " " << channel.getSlot()
                << " "  << channel.getChannel() << std::endl;
      if (hvmsg.isTurnon()) {
        //msg.setRampupSpeed(hvmsg.getRampupSpeed());
        //msg.setCommand(ArichHVMessage::RAMPUP_SPEED);
      } else if (hvmsg.isRampupSpeed()) {
        msg.setRampUpSpeed(hvmsg.getRampupSpeed());
        msg.setCommand(ArichHVMessage::RAMPUP_SPEED);
      } else if (hvmsg.isRampdownSpeed()) {
        msg.setRampDownSpeed(hvmsg.getRampdownSpeed());
        msg.setCommand(ArichHVMessage::RAMPDOWN_SPEED);
      } else if (hvmsg.isVoltageLimit()) {
        msg.setVoltageLimit(hvmsg.getVoltageLimit());
        msg.setCommand(ArichHVMessage::VOLTAGE_LIMIT);
      } else if (hvmsg.isCurrentLimit()) {
        msg.setCurrentLimit(hvmsg.getCurrentLimit());
        msg.setCommand(ArichHVMessage::CURRENT_LIMIT);
      } else if (hvmsg.isVoltageDemand()) {
        std::cout << hvmsg.getVoltageDemand() << std::endl;
        msg.setVoltageDemand(hvmsg.getVoltageDemand());
        msg.setCommand(ArichHVMessage::VOLTAGE_DEMAND);
      }
      m_comm_v[channel.getCrate()]->perform(msg, HVState(getNode().getState()));
    }
  }
  return true;
}
