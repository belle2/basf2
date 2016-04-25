#include "daq/slc/apps/logcollectord/LogcollectorCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DAQLogDB.h>

using namespace Belle2;

LogcollectorCallback::LogcollectorCallback(const std::string& name,
                                           DBInterface& db,
                                           int timeout)
  : NSMCallback(), m_db(db)
{
  setNode(NSMNode(name));
  setTimeout(timeout);
}

LogcollectorCallback::~LogcollectorCallback() throw()
{
}

void LogcollectorCallback::init(NSMCommunicator&) throw()
{

}

void LogcollectorCallback::timeout(NSMCommunicator&) throw()
{

}

void LogcollectorCallback::logset(const DAQLogMessage& msg) throw()
{
  LogFile::put(msg.getPriority(), msg.getNodeName() + " : " + msg.getMessage());
  try {
    if (!m_db.isConnected()) m_db.connect();
    DAQLogDB::createLog(m_db, m_logtable, msg);
  } catch (const DBHandlerException& e) {
    m_db.close();
    LogFile::error("DB errir : %s", e.what());
  }
  if (msg.getPriority() > LogFile::DEBUG) {
    for (NSMNodeList::iterator i = m_nodes.begin();
         i != m_nodes.end(); i++) {
      try {
        NSMCommunicator::send(NSMMessage(i->second, msg, NSMCommand::LOGSET));
      } catch (const NSMHandlerException& e) {
        LogFile::error(e.what());
      }
    }
    m_msgs.push_back(msg);
  }
  if (m_msgs.size() > 1000) {
    m_msgs.erase(m_msgs.begin());
  }
  while (m_msgs.size() > 0) {
    std::vector<DAQLogMessage>::iterator it = m_msgs.begin();
    if (Date().get() - it->getDateInt() > 3600 * 24) {
      m_msgs.erase(it);
      continue;
    }
    break;
  }
}

void LogcollectorCallback::logget(const std::string& nodename,
                                  LogFile::Priority pri) throw()
{
  NSMNode node(nodename);
  if (m_nodes.find(nodename) == m_nodes.end()) {
    LogFile::info("Added listner node " + nodename);
    m_nodes.insert(NSMNodeList::value_type(nodename, NSMNode(nodename)));
    m_pris.insert(PriorityList::value_type(nodename, pri));
    try {
      NSMCommunicator::send(NSMMessage(node, DAQLogMessage(getNode().getName(), LogFile::DEBUG,
                                                           "Registered in log collector"), NSMCommand::LOG));
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
    }
  } else {
    try {
      NSMCommunicator::send(NSMMessage(node, DAQLogMessage(getNode().getName(), LogFile::DEBUG,
                                                           "Registered again in log collector"), NSMCommand::LOG));
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
    }
  }
  try {
    for (std::vector<DAQLogMessage>::iterator it = m_msgs.begin();
         it != m_msgs.end(); it++) {
      DAQLogMessage& msg(*it);
      if (msg.getPriority() > LogFile::DEBUG) {
        NSMCommunicator::send(NSMMessage(node, *it, NSMCommand::LOGSET));
      }
    }
  } catch (const NSMHandlerException& e) {
    LogFile::error(e.what());
  }
}
