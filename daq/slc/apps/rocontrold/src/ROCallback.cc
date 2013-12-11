#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"

#include <cstdlib>

using namespace Belle2;

ROCallback::ROCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  //_man.setCallback(this);
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  //_man.create();
}

bool ROCallback::boot() throw()
{
  return true;
}

bool ROCallback::load() throw()
{
  download();
  /*
  _man.clearArguments();
  const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
  _man.setScriptDir(Belle2::form("%s/daq/copper/daq_scripts/", belle2_path));
  _man.setScript(_node->getData()->getText("script"));
  _man.addArgument("1");
  _man.addArgument("5101");
  _man.addArgument(_node->getName());
  return _man.load();
  */
  return false;
}

bool ROCallback::start() throw()
{
  return false;//_man.start();
}

bool ROCallback::stop() throw()
{
  return false;//_man.stop();
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
  return false;//_man.stop();
}

