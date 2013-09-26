#include "TemplateCallback.hh"

#include <nsm/RunStatus.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <unistd.h>

using namespace B2DAQ;

TemplateCallback::TemplateCallback(NSMNode* node)
  : RCCallback(node)
{
}

TemplateCallback::~TemplateCallback() throw()
{

}

bool TemplateCallback::boot() throw()
{
  B2DAQ::debug("BOOT");
  sleep(3);
  return true;
}

bool TemplateCallback::reboot() throw()
{
  B2DAQ::debug("REBOOT");
  sleep(3);
  return true;
}

bool TemplateCallback::load() throw()
{
  B2DAQ::debug("LOAD");
  sleep(3);
  return true;
}

bool TemplateCallback::reload() throw()
{
  B2DAQ::debug("RELOAD");
  sleep(3);
  return true;
}

bool TemplateCallback::start() throw()
{
  B2DAQ::debug("START");
  sleep(3);
  return true;
}

bool TemplateCallback::stop() throw()
{
  B2DAQ::debug("STOP");
  sleep(3);
  return true;
}

bool TemplateCallback::resume() throw()
{
  return true;
}

bool TemplateCallback::pause() throw()
{
  return true;
}

bool TemplateCallback::recover() throw()
{
  return true;
}

bool TemplateCallback::abort() throw()
{
  return true;
}
