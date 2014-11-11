#include "daq/slc/apps/eclcollectord/ECLCollectorCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

ECLCollectorCallback::ECLCollectorCallback(const NSMNode& node,
                                           const std::string& hostname)
  : RCCallback(node, 5), m_collector(hostname)
{
  //m_data = NSMData("STATUS_" + node.getName(), format, revision);

}

ECLCollectorCallback::~ECLCollectorCallback() throw()
{
}

void ECLCollectorCallback::init() throw()
{
  //m_data.allocate(getCommunicator());
}

void ECLCollectorCallback::term() throw()
{
  //m_collector.close();
}

void ECLCollectorCallback::timeout() throw()
{
}

bool ECLCollectorCallback::load() throw()
{
  m_collector.boot("");
  m_collector.initialize(2);
  return true;
}

bool ECLCollectorCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("run # = %04d.%04d.%03d",
                 msg.getParam(0), msg.getParam(1),
                 msg.getParam(2));
  return true;
}

bool ECLCollectorCallback::stop() throw()
{
  return true;
}

bool ECLCollectorCallback::resume() throw()
{
  return true;
}

bool ECLCollectorCallback::pause() throw()
{
  return true;
}

bool ECLCollectorCallback::recover() throw()
{
  getNode().setState(RCState::READY_S);
  return false;
}

bool ECLCollectorCallback::abort() throw()
{
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

