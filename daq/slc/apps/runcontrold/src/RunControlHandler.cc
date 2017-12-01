
#include "daq/slc/apps/runcontrold/RunControlHandler.h"

#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

bool NSMVHandlerRCConfig::handleGetText(std::string& val)
{
  try {
    m_callback.get(m_rcnode, "rcconfig", val);
    std::vector<std::string> s = StringUtil::split(val, '@');
    if (s.size() > 1) val = s[1];
    m_rcnode.setConfig(val);
    //val = m_rcnode.getConfig();
  } catch (const IOException& e) {
    LogFile::debug("%s : %s rcconfig", e.what(), m_rcnode.getName().c_str());
    val = "";
    m_rcnode.setConfig(val);
  }
  return true;
}

bool NSMVHandlerRCConfig::handleSetText(const std::string& val)
{
  /*
  m_callback.setConfig(m_rcnode, val);
  try {
    NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::CONFIGURE, val));
    m_callback.log(LogFile::INFO, "configuring node : %s (config=%s)", m_rcnode.getName().c_str(), val.c_str());
    return true;
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  */
  return false;
}

bool NSMVHandlerRCState::handleGetText(std::string& val)
{
  val = m_rcnode.getState().getLabel();
  return true;
}

bool NSMVHandlerRCRequest::handleGetText(std::string& val)
{
  get() >> val;
  return true;
}

bool NSMVHandlerRCRequest::handleSetText(const std::string& val)
{
  std::string str = StringUtil::toupper(StringUtil::replace(val, ":", "_"));
  if (!StringUtil::find(str, "RC_")) str = "RC_" + str;
  RCCommand cmd(str);
  try {
    NSMMessage msg(m_rcnode, cmd);
    if (cmd == RCCommand::CONFIGURE) {
      msg.setData(m_rcnode.getConfig());
    }
    if (!NSMCommunicator::send(msg)) {
      m_callback.log(LogFile::ERROR, "Failed to request %s to %s",
                     str.c_str(), m_rcnode.getName().c_str());
      return false;
    }
    RCState tstate = cmd.nextTState();
    if (tstate != Enum::UNKNOWN) {
      m_callback.setState(m_rcnode, tstate);
      if (tstate == RCState::ABORTING_RS) {
        m_callback.RCCallback::setState(RCState::NOTREADY_S);
      } else {
        m_callback.RCCallback::setState(tstate);
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool NSMVHandlerRCNodeUsed::handleGetInt(int& val)
{
  val = m_callback.getRCUsed() && m_rcnode.isUsed();
  return true;
}

bool NSMVHandlerRCNodeUsed::handleSetInt(int val)
{
  m_rcnode.setUsed(val > 0);
  if (val == 0) {
    m_callback.setState(m_rcnode, RCState::OFF_S);
    try {
      //NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::ABORT));
    } catch (const NSMHandlerException& e) {
      LogFile::warning(e.what());
    }
  } else {
    std::string state;
    try {
      m_callback.get(m_rcnode, "rcstate", state);
      m_callback.setState(m_rcnode, RCState(state));
    } catch (const NSMHandlerException& e) {
      LogFile::warning(e.what());
    } catch (const TimeoutException& e) {
      LogFile::warning(e.what());
      m_callback.setState(m_rcnode, NSMState::UNKNOWN);
    }
  }
  LogFile::info("set %s.used to %d", m_rcnode.getName().c_str(), val);
  return true;
}

bool NSMVHandlerRCUsed::handleSetInt(int val)
{
  return m_callback.setRCUsed(val);
}

bool NSMVHandlerRCExpNumber::handleSetInt(int val)
{
  if (val > 0) {
    m_callback.setExpNumber(val);
    return NSMVHandlerInt::handleSetInt(val);
  }
  return false;
}
