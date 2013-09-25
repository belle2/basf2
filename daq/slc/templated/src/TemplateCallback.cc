#include "TemplateCallback.hh"

#include <nsm/RunStatus.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

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
  return true;
}

bool TemplateCallback::reboot() throw()
{
  B2DAQ::debug("REBOOT");
  return true;
}

bool TemplateCallback::load() throw()
{
  B2DAQ::debug("LOAD");
  return true;
}

bool TemplateCallback::reload() throw()
{
  B2DAQ::debug("RELOAD");
  return true;
}

bool TemplateCallback::start() throw()
{
  B2DAQ::debug("START");
  return true;
}

bool TemplateCallback::stop() throw()
{
  B2DAQ::debug("STOP");
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
