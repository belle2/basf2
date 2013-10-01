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
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::load() throw()
{
  B2DAQ::debug("LOAD");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::start() throw()
{
  B2DAQ::debug("START");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  B2DAQ::debug("Exp no: %d", (int)getMessage().getParam(0));
  B2DAQ::debug("Run no: %d", (int)getMessage().getParam(1));
  return true;
}

bool TemplateCallback::stop() throw()
{
  B2DAQ::debug("STOP");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::resume() throw()
{
  B2DAQ::debug("RESUME");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::pause() throw()
{
  B2DAQ::debug("PAUSE");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::abort() throw()
{
  B2DAQ::debug("ABORT");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::trigft() throw()
{
  B2DAQ::debug("TRIGFT");
  B2DAQ::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    B2DAQ::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));

  return true;
}
