#include "TemplateCallback.h"

#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

TemplateCallback::TemplateCallback(NSMNode* node)
  : RCCallback(node)
{
  add(Command::TRIGFT);
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

