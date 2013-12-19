#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node, const std::string& dir)
  : RCCallback(node), _dir(dir)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
  _con.setCallback(this);
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  _con.init();
}

bool ROCallback::boot() throw()
{
  Belle2::debug("BOOT");
  return true;
}

bool ROCallback::load() throw()
{
  Belle2::debug("LOAD");
  download();
  _con.clearArguments();
  _con.addArgument(_dir + _node->getData()->getText("script"));
  _con.addArgument("1");
  _con.addArgument("5101");
  _con.addArgument(_node->getName());
  if (_con.load(10)) {
    Belle2::debug("(DEBUG) load succeded");
  } else {
    Belle2::debug("(DEBUG) load timeout");
  }
  return true;
}

bool ROCallback::start() throw()
{
  Belle2::debug("START");
  if (_con.start(10)) {
    Belle2::debug("(DEBUG) start succeded");
  } else {
    Belle2::debug("(DEBUG) start timeout");
  }
  return true;
}

bool ROCallback::stop() throw()
{
  Belle2::debug("STOP");
  return _con.stop(30);
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
  Belle2::debug("RECOVER");
  _node->setState(State::CONFIGURED_S);
  return (_con.abort() && load());
}

bool ROCallback::abort() throw()
{
  Belle2::debug("ABORT");
  return _con.abort();
}

