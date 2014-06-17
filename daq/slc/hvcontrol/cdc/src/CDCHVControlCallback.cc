#include "daq/slc/hvcontrol/cdc/CDCHVControlCallback.h"

#include <daq/slc/hvcontrol/cdc/CDCHVCommunicator.h>
#include <daq/slc/hvcontrol/cdc/CDCHVMonitor.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <iostream>

using namespace Belle2;

CDCHVControlCallback::~CDCHVControlCallback() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    delete m_comm_v[i];
  }
}

void CDCHVControlCallback::initialize() throw()
{
  ConfigFile config("hvcontrol/cdc");
  const int ncrates = config.getInt("cdc.hv.ncrates");
  for (int n = 0; n < ncrates; n++) {
    const std::string host = config.get(StringUtil::form("cdc.hv.%d.host", n));
    int port = config.getInt(StringUtil::form("cdc.hv.%d.port", n));
    CDCHVCommunicator* comm = new CDCHVCommunicator(this, n, host, port);
    PThread(new CDCHVMonitor(comm));
    m_comm_v.push_back(comm);
  }
}

bool CDCHVControlCallback::configure() throw()
{
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    m_comm_v[i]->configure();
  }
  return true;
}

bool CDCHVControlCallback::turnon() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnon();
  }
  return success;
}

bool CDCHVControlCallback::turnoff() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->turnoff();
  }
  return success;
}

bool CDCHVControlCallback::standby() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->standby();
  }
  return success;
}

bool CDCHVControlCallback::shoulder() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->shoulder();
  }
  return success;
}

bool CDCHVControlCallback::peak() throw()
{
  bool success = true;
  for (size_t i = 0; i < m_comm_v.size(); i++) {
    success &= m_comm_v[i]->peak();
  }
  return success;
}

bool CDCHVControlCallback::hvapply(const HVApplyMessage& hvmsg)
throw()
{
  std::cout << hvmsg.getCrate() << " " << hvmsg.getSlot()
            << " "  << hvmsg.getChannel() << std::endl;
  for (size_t i = 0; i < getNChannels(); i++) {
    const HVChannel& channel(getConfig().getChannel(i));
    if ((hvmsg.isAllCrates() || hvmsg.getCrate() == channel.getCrate()) &&
        (hvmsg.isAllSlots() || hvmsg.getSlot() == channel.getSlot()) &&
        (hvmsg.isAllChannels() || hvmsg.getChannel() == channel.getChannel())) {
      CDCHVMessage msg(CDCHVMessage::SET, CDCHVMessage::ALL,
                       /*channel.getSlot(), */channel.getChannel());
      std::cout << i << " " << channel.getCrate() << " " << channel.getSlot()
                << " "  << channel.getChannel() << std::endl;
      if (hvmsg.isTurnon()) {
        //msg.setRampupSpeed(hvmsg.getRampupSpeed());
        //msg.setCommand(CDCHVMessage::RAMPUP_SPEED);
      } else if (hvmsg.isRampupSpeed()) {
        msg.setRampUpSpeed(hvmsg.getRampupSpeed());
        msg.setCommand(CDCHVMessage::RAMPUP_SPEED);
      } else if (hvmsg.isRampdownSpeed()) {
        msg.setRampDownSpeed(hvmsg.getRampdownSpeed());
        msg.setCommand(CDCHVMessage::RAMPDOWN_SPEED);
      } else if (hvmsg.isVoltageLimit()) {
        msg.setVoltageLimit(hvmsg.getVoltageLimit());
        msg.setCommand(CDCHVMessage::VOLTAGE_LIMIT);
      } else if (hvmsg.isCurrentLimit()) {
        msg.setCurrentLimit(hvmsg.getCurrentLimit());
        msg.setCommand(CDCHVMessage::CURRENT_LIMIT);
      } else if (hvmsg.isVoltageDemand()) {
        std::cout << hvmsg.getVoltageDemand() << std::endl;
        msg.setVoltageDemand(hvmsg.getVoltageDemand());
        msg.setCommand(CDCHVMessage::VOLTAGE_DEMAND);
      }
      m_comm_v[channel.getCrate()]->perform(msg, HVState(getNode().getState()));
    }
  }
  return true;
}
