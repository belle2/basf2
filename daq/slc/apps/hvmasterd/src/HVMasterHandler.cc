#include "daq/slc/apps/hvmasterd/HVMasterHandler.h"
#include "daq/slc/apps/hvmasterd/HVMasterCallback.h"

#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

bool NSMVHandlerHVConfig::handleGetText(std::string& val)
{
  try {
    m_callback.get(m_hvnode, "config", val);
    m_hvnode.setConfig(val);
  } catch (const IOException& e) {
    LogFile::error("%s : %s config", e.what(),
                   m_hvnode.getName().c_str());
    return false;
  }
  return true;
}

bool NSMVHandlerHVConfig::handleSetText(const std::string& val)
{
  m_callback.setConfig(m_hvnode, val);
  try {
    NSMCommunicator::send(NSMMessage(m_hvnode, HVCommand::CONFIGURE, val));
    LogFile::info("configured node : %s (config=%s)",
                  m_hvnode.getName().c_str(), val.c_str());
    return true;
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return false;
}

bool NSMVHandlerHVNodeState::handleGetText(std::string& val)
{
  val = m_hvnode.getState().getLabel();
  return true;
}

bool NSMVHandlerHVRequest::handleGetText(std::string& val)
{
  get() >> val;
  return true;
}

bool NSMVHandlerHVRequest::handleSetText(const std::string& val)
{
  std::string str = StringUtil::toupper(StringUtil::replace(val, ":", "_"));
  if (!StringUtil::find(str, "HV_")) str = "HV_" + str;
  HVCommand cmd(str);
  LogFile::debug("hvrequest %s to %s", str.c_str(), m_hvnode.getName().c_str());
  try {
    NSMMessage msg(m_hvnode, cmd);
    if (cmd == HVCommand::CONFIGURE) {
      msg.setData(m_hvnode.getConfig());
    }
    if (!NSMCommunicator::send(msg)) {
      LogFile::error("Failed to request %s to %s",
                     str.c_str(), m_hvnode.getName().c_str());
      return false;
    }
    HVState tstate = cmd.nextTState(m_hvnode.getState());
    if (tstate != Enum::UNKNOWN) {
      m_callback.setHVState(m_hvnode, tstate);
      //m_callback.HVCallback::setState(tstate);
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool NSMVHandlerHVNodeUsed::handleGetInt(int& val)
{
  val = m_hvnode.isUsed();
  return true;
}

bool NSMVHandlerHVNodeUsed::handleSetInt(int val)
{
  m_hvnode.setUsed(val > 0);
  if (val > 0) {
    std::string state;
    try {
      m_callback.get(m_hvnode, "hvstate", state);
      m_callback.setHVState(m_hvnode, HVState(state));
    } catch (const NSMHandlerException& e) {
      LogFile::warning(e.what());
    } catch (const TimeoutException& e) {
      LogFile::warning(e.what());
      m_callback.setHVState(m_hvnode, NSMState::UNKNOWN);
    }
  }
  LogFile::info("set %s.used to %d", m_hvnode.getName().c_str(), val);
  return true;
}


