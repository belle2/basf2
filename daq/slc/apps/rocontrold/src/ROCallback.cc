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
  m_con.init("basf2");
}

void ROCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool ROCallback::boot() throw()
{
  return true;
}

bool ROCallback::load() throw()
{
  const DBObject& obj(getConfig().getObject());
  std::string script = obj.getText("script");
  m_con.clearArguments();
  m_con.addArgument(script);
  m_con.addArgument("1");
  m_con.addArgument("5101");
  m_con.addArgument("basf2");
  if (m_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  } else {
  }
  LogFile::error("load timeout");
  return false;
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
  if (abort() && boot() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool ROCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::INITIAL_S);
  return true;
}

