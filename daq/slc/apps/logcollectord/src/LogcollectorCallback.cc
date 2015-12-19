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
  for (NSMNodeList::iterator i = m_nodes.begin();
       i != m_nodes.end(); i++) {
    try {
      NSMCommunicator::send(NSMMessage(i->second, msg, NSMCommand::LOGSET));
    } catch (const NSMHandlerException& e) {
      LogFile::error(e.what());
    }
  }
}

void LogcollectorCallback::logget(const std::string& nodename,
                                  LogFile::Priority pri) throw()
{
  if (m_nodes.find(nodename) == m_nodes.end()) {
    LogFile::info("Added listner node " + nodename);
    m_nodes.insert(NSMNodeList::value_type(nodename, NSMNode(nodename)));
    m_pris.insert(PriorityList::value_type(nodename, pri));
    return;
  }
}
