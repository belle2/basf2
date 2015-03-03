#include "daq/slc/apps/runcontrold/RunControlHandler.h"

#include "daq/slc/apps/runcontrold/RunControlCallback.h"

#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

bool NSMVHandlerRCConfig::handleGetText(std::string& val)
{
  val = m_rcnode.getConfig();
  return true;
}

bool NSMVHandlerRCConfig::handleSetText(const std::string& val)
{
  m_rcnode.setConfig(val);
  return true;
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
  std::string str = StringUtil::toupper(val);
  if (!StringUtil::find(str, "RC_")) str = "RC_" + str;
  RCCommand cmd(str);
  LogFile::debug("rcrequest %s to %s", str.c_str(), m_rcnode.getName().c_str());
  try {
    NSMMessage msg(m_rcnode, cmd);
    if (cmd == RCCommand::CONFIGURE) {
      msg.setData(m_rcnode.getConfig());
    }
    if (!NSMCommunicator::send(msg)) {
      LogFile::error("Failed to request %s to %s",
                     str.c_str(), m_rcnode.getName().c_str());
      return false;
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool NSMVHandlerRCUsed::handleGetInt(int& val)
{
  val = m_rcnode.isUsed();
  return true;
}

bool NSMVHandlerRCUsed::handleSetInt(int val)
{
  m_rcnode.setUsed(val > 0);
  return true;
}

bool NSMVHandlerRCExcluded::handleGetInt(int& val)
{
  val = m_rcnode.isExcluded();
  return true;
}

bool NSMVHandlerRCExcluded::handleSetInt(int val)
{
  m_rcnode.setExcluded(val > 0);
  return true;
}
