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
  B2DAQ::debug("RESUME");
  return true;
}

bool TemplateCallback::pause() throw()
{
  B2DAQ::debug("PAUSE");
  return true;
}

bool TemplateCallback::abort() throw()
{
  B2DAQ::debug("ABORT");
  return true;
}

bool TemplateCallback::trigft() throw()
{
  B2DAQ::debug("TRIGFT");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  B2DAQ::debug("pars[0]: %d", (int)getMessage().getParam(0));
  B2DAQ::debug("pars[1]: %d", (int)getMessage().getParam(1));
  B2DAQ::debug("pars[2]: %d", (int)getMessage().getParam(2));
  B2DAQ::debug("pars[3]: %d", (int)getMessage().getParam(3));
  B2DAQ::debug("pars[4]: %d", (int)getMessage().getParam(4));

  return true;
}
