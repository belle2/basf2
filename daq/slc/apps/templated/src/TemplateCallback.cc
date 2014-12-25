#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

TemplateCallback::TemplateCallback(const NSMNode& node/*,
                                   const std::string& format,
                                   int revision*/)
  : RCCallback(node, 5)
{
  //m_data = NSMData("STATUS_" + node.getName(), format, revision);
}

TemplateCallback::~TemplateCallback() throw()
{
}

void TemplateCallback::init() throw()
{
  //m_data.allocate(getCommunicator());
}

void TemplateCallback::term() throw()
{
}

void TemplateCallback::timeout() throw()
{
}

bool TemplateCallback::load() throw()
{
  sleep(5);
  getConfig().getObject().print();
  return true;
}

bool TemplateCallback::trigft() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("trigger type  : %d", msg.getParam(0));
  LogFile::debug("dummy rate    : %d", msg.getParam(1));
  LogFile::debug("trigger limit : %d", msg.getParam(2));
  return true;
}

bool TemplateCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("run # = %04d.%04d.%03d",
                 msg.getParam(0), msg.getParam(1),
                 msg.getParam(2));
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
  getNode().setState(RCState::READY_S);
  return false;
}

bool TemplateCallback::abort() throw()
{
  sleep(5);
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

