#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

NSM2SocketCallback::NSM2SocketCallback(const NSMNode& node, int interval,
                                       ConfigFile& file)
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
  std::vector<std::string> datname_v = StringUtil::split(file.get("nsmdata"), ',');
  for (size_t i = 0; i < datname_v.size(); i++) {
    std::string dataname = datname_v[i];
    std::string format = file.get(StringUtil::form("%s.format", dataname.c_str()));
    int revision = file.getInt(StringUtil::form("%s.revision", dataname.c_str()));
    m_data_v.push_back(NSMData(dataname, format, revision));
  }
}

bool NSM2SocketCallback::perform(NSMMessage& msg)
throw(NSMHandlerException)
{
  m_bridge->sendMessage(msg);
  return true;
}

void NSM2SocketCallback::init() throw()
{
  NSMCommunicator* comm = getCommunicator();
  for (NSMDataList::iterator it = m_data_v.begin();
       it != m_data_v.end(); it++) {
    NSMData& data(*it);
    try {
      std::cout << "open NSM data (" << data.getName() << ")" << std::endl;
      data.open(comm);
    } catch (const NSMHandlerException& e) {
      std::cout << e.what() << std::endl;
    }
  }
}

void NSM2SocketCallback::timeout() throw()
{
  NSMCommunicator* comm = getCommunicator();
  for (NSMDataList::iterator it = m_data_v.begin();
       it != m_data_v.end(); it++) {
    NSMData data(*it);
    if (!data.isAvailable()) {
      try {
        data.open(comm);
      } catch (const NSMHandlerException& e) {
        std::cout << e.what() << std::endl;
      }
    }
  }
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
