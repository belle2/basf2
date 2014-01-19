#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node, const std::string& configname)
  : RCCallback(node)
{
  node->setData(new DataObject());
  _con.setCallback(this);
  ConfigFile config(configname);
  _path = config.get("ROPC_BASF2_SCRIPT");
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
  LogFile::debug("load timeout");
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
    _node->setState(State::READY_S);
    return true;
  }
  return false;
}

bool ROCallback::abort() throw()
{
  _con.abort();
  _node->setState(State::INITIAL_S);
  return true;
}

