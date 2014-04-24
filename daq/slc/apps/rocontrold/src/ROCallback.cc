#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& node,
                       const std::string& configname)
  : RCCallback(node)
{
  _con.setCallback(this);
  ConfigFile config(configname);
  _path = config.get("rocp.basf2.dir");
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  _con.init("basf2");
}

void ROCallback::term() throw()
{
  _con.abort();
  _con.getInfo().unlink();
}

bool ROCallback::boot() throw()
{
  return true;
}

bool ROCallback::load() throw()
{
  _con.clearArguments();
  _con.addArgument(_path);
  _con.addArgument("1");
  _con.addArgument("5101");
  _con.addArgument("basf2");
  if (_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  } else {
  }
  LogFile::error("load timeout");
  return false;
}

bool ROCallback::start() throw()
{
  _con.start();
  return true;
}

bool ROCallback::stop() throw()
{
  _con.stop();
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
  _con.abort();
  getNode().setState(RCState::INITIAL_S);
  return true;
}

