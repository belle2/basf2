#include "TemplateCallback.h"

#include <nsm/RunStatus.h>

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
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::load() throw()
{
  Belle2::debug("LOAD");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::start() throw()
{
  Belle2::debug("START");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  Belle2::debug("Exp no: %d", (int)getMessage().getParam(0));
  Belle2::debug("Run no: %d", (int)getMessage().getParam(1));
  return true;
}

bool TemplateCallback::stop() throw()
{
  Belle2::debug("STOP");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::resume() throw()
{
  Belle2::debug("RESUME");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::pause() throw()
{
  Belle2::debug("PAUSE");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::abort() throw()
{
  Belle2::debug("ABORT");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));
  return true;
}

bool TemplateCallback::trigft() throw()
{
  Belle2::debug("TRIGFT");
  Belle2::debug("npar : %d", (int)getMessage().getNParams());
  for (size_t i = 0; i < getMessage().getNParams(); i++)
    Belle2::debug("pars[%d]: %d", (int)i, (int)getMessage().getParam(i));

  return true;
}
