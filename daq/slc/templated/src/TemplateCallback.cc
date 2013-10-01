#include "TemplateCallback.hh"

#include <nsm/RunStatus.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <unistd.h>

using namespace B2DAQ;

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
  B2DAQ::debug("BOOT");
  sleep(3);
  return true;
}

bool TemplateCallback::load() throw()
{
  B2DAQ::debug("LOAD");
  sleep(3);
  return true;
}

bool TemplateCallback::start() throw()
{
  B2DAQ::debug("START");
  B2DAQ::debug("Exp no: %d", (int)getMessage().getParam(0));
  B2DAQ::debug("Run no: %d", (int)getMessage().getParam(1));
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

bool TemplateCallback::abort() throw()
{
  return true;
}
