#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

TemplateCallback::TemplateCallback()
{
}

TemplateCallback::~TemplateCallback() throw()
{
}

void TemplateCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
}

void TemplateCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  if (obj.hasObject("value")) {
    const DBObject& o_value(obj("value"));
    if (o_value.hasValue("ival")) {
      LogFile::debug("ival=%d", o_value.getInt("ival"));
    }
    if (o_value.hasValue("bval")) {
      LogFile::debug("bval=%s", (o_value.getBool("bval") ? "true" : "false"));
    }
    if (o_value.hasValue("fval")) {
      LogFile::debug("fval=%d", o_value.getFloat("fval"));
    }
    if (o_value.hasText("text")) {
      LogFile::debug("text=%d", o_value.getText("text").c_str());
    }
  }
}

void TemplateCallback::timeout(NSMCommunicator&) throw()
{
}

void TemplateCallback::load(const DBObject& obj) throw(RCHandlerException)
{
}

void TemplateCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
}

void TemplateCallback::stop() throw(RCHandlerException)
{
}

void TemplateCallback::resume(int subno) throw(RCHandlerException)
{
  LogFile::debug("sub run # = %03d", subno);
}

void TemplateCallback::pause() throw(RCHandlerException)
{
}

void TemplateCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
}

void TemplateCallback::abort() throw(RCHandlerException)
{
}

