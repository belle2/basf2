#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

TemplateCallback::TemplateCallback(const NSMNode& node)
  : RCCallback(node)
{
}

TemplateCallback::~TemplateCallback() throw()
{
}

void TemplateCallback::init() throw()
{
  NSMData data;
  if (getNode().getName() == "ROPC01") {
    data = NSMData("STATUS_" + getNode().getName(), "ropc_status", 1);
  } else {
    data = NSMData("STATUS_" + getNode().getName(), "cpr_status", 1);
  }
  data.allocate(getCommunicator());
}

void TemplateCallback::term() throw()
{
}

bool TemplateCallback::boot() throw()
{
  return true;
}

bool TemplateCallback::load() throw()
{
  getConfig().getObject().print();
  return true;
}

bool TemplateCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  std::cout << msg.getNParams() << std::endl;
  std::cout << msg.getParam(0) << std::endl;
  std::cout << msg.getParam(1) << std::endl;
  std::cout << msg.getParam(2) << std::endl;
  return true;
}

bool TemplateCallback::stop() throw()
{
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
  if (abort() && boot() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool TemplateCallback::abort() throw()
{
  _con.abort();
  getNode().setState(RCState::INITIAL_S);
  return true;
}

