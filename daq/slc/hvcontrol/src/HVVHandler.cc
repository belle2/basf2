#include "daq/slc/hvcontrol/HVVHandler.h"

#include <daq/slc/hvcontrol/HVCallback.h>
#include <daq/slc/hvcontrol/HVCommand.h>
#include <daq/slc/hvcontrol/HVState.h>
#include <daq/slc/hvcontrol/HVMessage.h>

#include <daq/slc/nsm/NSMCommunicator.h>

using namespace Belle2;

#define HV_HANDLE_PRE       \
  m_callback.lock();        \
  try {

#define HV_HANDLE_POST              \
  } catch (const IOException& e) {          \
    LogFile::error(e.what());           \
    m_callback.reply(NSMMessage(NSMCommand::ERROR, e.what()));    \
    m_callback.unlock();            \
    return false;             \
  }                 \
  m_callback.unlock();              \
  return true

bool NSMVHandlerHVSwitch::handleGetText(std::string& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getSwitch(m_crate, m_slot, m_channel) ? "ON" : "OFF";
  HV_HANDLE_POST;
}

bool NSMVHandlerHVRampUpSpeed::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getRampUpSpeed(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVRampDownSpeed::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getRampDownSpeed(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageDemand::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getVoltageDemand(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageLimit::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getVoltageLimit(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVCurrentLimit::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getCurrentLimit(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVState::handleGetText(std::string& val)
{
  HV_HANDLE_PRE;
  val = HVMessage::getStateText((HVMessage::State)m_callback.getState(m_crate, m_slot, m_channel));
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageMonitor::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getVoltageMonitor(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVCurrentMonitor::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getCurrentMonitor(m_crate, m_slot, m_channel);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVSwitch::handleSetText(const std::string& val)
{
  HV_HANDLE_PRE;
  m_callback.setSwitch(m_crate, m_slot, m_channel, (val == "ON"));
  std::string state = HVMessage::getStateText((HVMessage::State)m_callback.getState(m_crate, m_slot, m_channel));
  std::string vname = StringUtil::replace(getName(), "switch", "state");
  m_callback.set(vname, state);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVRampUpSpeed::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setRampUpSpeed(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVRampDownSpeed::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setRampDownSpeed(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageDemand::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setVoltageDemand(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageLimit::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setVoltageLimit(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVCurrentLimit::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setCurrentLimit(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVState::handleSetText(const std::string& val)
{
  HV_HANDLE_PRE;
  m_callback.setState(m_crate, m_slot, m_channel, HVMessage::getState(val.c_str()));
  HV_HANDLE_POST;
}

bool NSMVHandlerHVVoltageMonitor::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setVoltageMonitor(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVCurrentMonitor::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setCurrentMonitor(m_crate, m_slot, m_channel, val);
  HV_HANDLE_POST;
}

