#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& node)
  : RCCallback(node)
{
  m_con.setCallback(this);
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  m_con.init("basf2_ropc");
}

void ROCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool ROCallback::load() throw()
{
  const DBObject& obj(getConfig().getObject());
  std::string script = obj.getText("ropc_script");
  m_con.setExecutable("basf2");
  m_con.clearArguments();
  m_con.addArgument(script);
  m_con.addArgument("1");
  m_con.addArgument(StringUtil::form("%d", obj.getInt("port_from")));
  m_con.addArgument("basf2_ropc");
  if (m_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  }
  LogFile::error("load timeout");
  //return false;
  return true;
}

bool ROCallback::start() throw()
{
  m_con.start();
  return true;
}

bool ROCallback::stop() throw()
{
  m_con.stop();
  return true;
}

bool ROCallback::resume() throw()
{
  return true;
}

bool ROCallback::pause() throw()
{
  return true;
}

bool ROCallback::recover() throw()
{
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool ROCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

