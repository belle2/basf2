#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

NSM2SocketCallback::NSM2SocketCallback(const NSMNode& node, int interval)
throw() : NSMCallback(node, interval)
{
  add(RCCommand::BOOT);
  add(RCCommand::LOAD);
  add(RCCommand::START);
  add(RCCommand::STOP);
  add(RCCommand::RECOVER);
  add(RCCommand::RESUME);
  add(RCCommand::PAUSE);
  add(RCCommand::ABORT);
  add(RCCommand::STATE);
  add(RCCommand::STATECHECK);
  add(RCCommand::TRIGFT);
  add(HVCommand::CONFIGURE);
  add(HVCommand::STANDBY);
  add(HVCommand::SHOULDER);
  add(HVCommand::PEAK);
  add(HVCommand::TURNON);
  add(HVCommand::TURNOFF);
}

bool NSM2SocketCallback::perform(NSMMessage& msg)
throw(NSMHandlerException)
{
  m_bridge->sendMessage(msg);
  return true;
}

void NSM2SocketCallback::init() throw()
{
}

NSMData& NSM2SocketCallback::getData(const std::string& name,
                                     const std::string& format,
                                     int revision) throw(NSMHandlerException)
{
  if (m_data_m.find(name) == m_data_m.end()) {
    NSMData data(name, format, revision);
    data.open(getCommunicator());
    LogFile::debug("open NSM data (%s) ", data.getName().c_str());
    m_data_m.insert(NSMDataList::value_type(name, data));
  }
  return m_data_m[name];
}

void NSM2SocketCallback::timeout() throw()
{

}

bool NSM2SocketCallback::sendRequest(NSMMessage& msg) throw()
{
  m_mutex.lock();
  try {
    getCommunicator()->sendRequest(msg);
  } catch (const NSMHandlerException& e) {
    m_mutex.unlock();
    return false;
  }
  m_mutex.unlock();
  return true;
}
