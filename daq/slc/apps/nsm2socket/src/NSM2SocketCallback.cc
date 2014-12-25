#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"

#include <daq/slc/runcontrol/RCCommand.h>
#include <daq/slc/hvcontrol/HVCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

NSM2SocketCallback::NSM2SocketCallback(const NSMNode& node)
throw() : NSMCallback(node, 1)
{
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
  add(HVCommand::HVAPPLY);
  setTimeout(1);
}

bool NSM2SocketCallback::perform(const NSMMessage& msg) throw()
{
  m_bridge->sendMessage(msg);
  return true;
}

NSMData& NSM2SocketCallback::getData(const std::string& name,
                                     const std::string& format,
                                     int revision) throw(NSMHandlerException)
{
  if (m_data_m.find(name) == m_data_m.end()) {
    NSMData data(name, format, revision);
    m_data_m.insert(NSMDataList::value_type(name, data));
  }
  return m_data_m[name];
}

bool NSM2SocketCallback::sendRequest(NSMMessage& msg) throw()
{
  m_mutex.lock();
  std::string nodename = msg.getNodeName();
  try {
    bool isonline = true;
    if (m_node_m.find(nodename) == m_node_m.end()) {
      isonline = getCommunicator()->isConnected(nodename);
      m_node_m.insert(std::map<std::string, bool>::value_type(nodename, isonline));
    } else {
      isonline = m_node_m[nodename];
    }
    if (!isonline) {
      m_bridge->sendLog(DAQLogMessage(getNode().getName(), LogFile::ERROR,
                                      StringUtil::form("Node %s was aready down.",
                                                       nodename.c_str())));
    } else {
      getCommunicator()->sendRequest(msg);
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
    m_bridge->sendLog(DAQLogMessage(getNode().getName(),
                                    LogFile::ERROR, e.what()));

    m_mutex.unlock();
    return false;
  }
  m_mutex.unlock();
  return true;
}

void NSM2SocketCallback::timeout() throw()
{
  static unsigned long long n = 0;
  for (std::map<std::string, bool>::iterator it = m_node_m.begin();
       it != m_node_m.end(); it++) {
    std::string nodename = it->first;
    bool isonline = it->second;
    bool isonline2 = getCommunicator()->isConnected(nodename);
    m_node_m[nodename] = isonline2;
    if (isonline && !isonline2) {
      LogFile::error("Node %s got down.", nodename.c_str());
      m_bridge->sendLog(DAQLogMessage(getNode().getName(), LogFile::ERROR,
                                      StringUtil::form("Node %s got down.",
                                                       nodename.c_str())));
    } else if (!isonline && isonline2) {
      LogFile::notice("Node %s got up.", nodename.c_str());
      m_bridge->sendLog(DAQLogMessage(getNode().getName(), LogFile::NOTICE,
                                      StringUtil::form("Node %s got up.",
                                                       nodename.c_str())));
    }
  }

  for (NSMDataList::iterator it = m_data_m.begin();
       it != m_data_m.end(); it++) {
    NSMData& data(it->second);
    NSMMessage msg(getNode());
    try {
      bool opennew = false;
      if (!data.isAvailable()) {
        data.open(getCommunicator());
        LogFile::info("open NSM data (%s) ", data.getName().c_str());
        opennew = true;
      }
      if (n % 5 == 0 || opennew) {
        msg.setNodeName(data.getName());
        msg.setRequestName(NSMCommand::NSMSET);
        data.update();
        m_bridge->sendMessage(msg, data);
      }
    } catch (const NSMHandlerException& e) {
      std::string log = StringUtil::form("Failed to open NSM data (%s:%s:%d) ",
                                         data.getName().c_str(),
                                         data.getFormat().c_str(),
                                         data.getRevision());
      LogFile::error(e.what());
      LogFile::error(log);
      m_bridge->sendLog(DAQLogMessage(getNode().getName(),
                                      LogFile::ERROR, log));
    }
  }
  n++;
}

