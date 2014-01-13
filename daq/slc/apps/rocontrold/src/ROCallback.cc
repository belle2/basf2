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
  node->setState(State::INITIAL_S);
  _con.setCallback(this);
  ConfigFile config(configname);
  _path = config.get("ROPC_BASF2_PATH");
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
  if (_con.load(20)) {
    LogFile::debug("load succeded");
  } else {
    LogFile::debug("load timeout");
  }
  return true;
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
  return (abort() && boot() && load());
}

bool ROCallback::abort() throw()
{
  return _con.abort();
}

