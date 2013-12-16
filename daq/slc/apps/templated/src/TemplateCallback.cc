#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/State.h>

#include <unistd.h>

using namespace Belle2;

TemplateCallback::TemplateCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
}

TemplateCallback::~TemplateCallback() throw()
{

}

bool TemplateCallback::boot() throw()
{
  Belle2::debug("BOOT");
  return true;
}

bool TemplateCallback::load() throw()
{
  Belle2::debug("LOAD");
  download();
  _node->getData()->print();
  return true;
}

bool TemplateCallback::start() throw()
{
  Belle2::debug("START");
  Belle2::debug("Exp no: %d", (int)getMessage().getParam(0));
  Belle2::debug("Run no: %d", (int)getMessage().getParam(1));
  return true;
}

bool TemplateCallback::stop() throw()
{
  Belle2::debug("STOP");
  return true;
}

bool TemplateCallback::recover() throw()
{
  Belle2::debug("RECOVER");
  return true;
}

bool TemplateCallback::abort() throw()
{
  Belle2::debug("ABORT");
  return true;
}

bool TemplateCallback::trigft() throw()
{
  Belle2::debug("TRIGFT");
  Belle2::debug("trigger_mode  : %d", (int)getMessage().getParam(0));
  Belle2::debug("dummy_rate    : %d", (int)getMessage().getParam(1));
  Belle2::debug("trigger_limit : %d", (int)getMessage().getParam(2));
  return true;
}

