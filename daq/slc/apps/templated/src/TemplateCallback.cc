#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/readout/ReadoutMonitor.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

TemplateCallback::TemplateCallback(const NSMNode& node,
                                   const std::string& host,
                                   const std::string& port)
  : RCCallback(node), m_con(this)
{
  m_host = host;
  m_port = port;
}

TemplateCallback::~TemplateCallback() throw()
{
}

void TemplateCallback::init() throw()
{
  m_data = NSMData("STATUS_" + getNode().getName(), "ronode_status", 1);
  m_data.allocate(getCommunicator());
}

void TemplateCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool TemplateCallback::load() throw()
{
  getConfig().getObject().print();
  std::string workername = "woerker:" + getNode().getName();
  m_con.init(workername, sizeof(ronode_info));
  m_con.clearArguments();
  if (getNode().getName().find("CPR") != std::string::npos) {
    m_con.setExecutable("rodummy_out");
  } else {
    m_con.setExecutable("rodummy_in");
  }
  m_con.addArgument(workername);
  m_con.addArgument("1");
  m_con.addArgument(m_host);
  m_con.addArgument(m_port);
  m_con.load(-1);
  m_thread = PThread(new ReadoutMonitor(this,
                                        (ronode_info*)m_con.getInfo().getReserved(),
                                        (ronode_status*)m_data.get()));
  return true;
}

bool TemplateCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("run # = %04d.%04d.%03d",
                 msg.getParam(0), msg.getParam(1),
                 msg.getParam(2));
  m_con.start();
  return true;
}

bool TemplateCallback::stop() throw()
{
  m_con.stop();
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
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool TemplateCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

