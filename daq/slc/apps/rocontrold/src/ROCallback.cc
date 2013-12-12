#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <cstdlib>

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node)
  : RCCallback(node)
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
  return true;
}

bool ROCallback::load() throw()
{
  download();
  _con.clearArguments();
  _con.setScriptDir("daq/copper/daq_scripts");
  _con.setScript(_node->getData()->getText("script"));
  _con.addArgument("1");
  _con.addArgument("5101");
  _con.addArgument(_node->getName());
  return _con.load(10);
}

bool ROCallback::start() throw()
{
  return _con.start(10);
}

bool ROCallback::stop() throw()
{
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

bool ROCallback::abort() throw()
{
  return _con.abort();
}

