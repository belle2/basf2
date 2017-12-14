#include "daq/slc/apps/logmqd/LogMQCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DAQLogDB.h>

using namespace Belle2;
using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace cms;

LogMQCallback::LogMQCallback(const std::string& name,
                             DBInterface& db,
                             int timeout)
  : NSMCallback(), m_db(db)
{
  setNode(NSMNode(name));
  setTimeout(timeout);

  activemq::library::ActiveMQCPP::initializeLibrary();
  m_connectionFactory = new ActiveMQConnectionFactory("tcp://belle-daq:61616");
  m_connection = m_connectionFactory->createConnection();
  m_connection->start();
  m_session = m_connection->createSession(Session::AUTO_ACKNOWLEDGE);
  m_destination = m_session->createTopic("LOG");
  m_producer = m_session->createProducer(m_destination);
}

LogMQCallback::~LogMQCallback() throw()
{
  delete m_destination;
  delete m_session;
  delete m_connection;
  delete m_connectionFactory;
}

void LogMQCallback::init(NSMCommunicator&) throw()
{
}

void LogMQCallback::timeout(NSMCommunicator&) throw()
{

}

void LogMQCallback::logset(const DAQLogMessage& msg) throw()
{
  LogFile::put(msg.getPriority(), msg.getNodeName() + " : " + msg.getMessage());
  try {
    if (!m_db.isConnected()) m_db.connect();
    DAQLogDB::createLog(m_db, m_logtable, msg);
  } catch (const DBHandlerException& e) {
    m_db.close();
    LogFile::error("DB errir : %s", e.what());
  }

  try {
    MapMessage* m = (MapMessage*)m_session->createMapMessage();
    m->setString("TYPE", "log");
    m->setString("TEXT", msg.getMessage());
    //m->setString("CONFIG", "log");
    m->setString("NAME", msg.getNodeName());
    //m->setString("CURRENT_SEVERITY", "OK");
    m->setString("STATE", msg.getPriorityText());
    std::string s = "DEBUG";
    switch (msg.getPriority()) {
      case LogFile::DEBUG:   s = "DEBUG"; break;
      case LogFile::INFO:    s = "INFO";  break;
      case LogFile::NOTICE:
      case LogFile::WARNING: s = "WARN"; break;
      case LogFile::ERROR: s = "ERROR"; break;
      case LogFile::FATAL: s = "FATAL"; break;
      default:
        break;
    }
    m->setString("SEVERITY", s);
    m->setString("CREATETIME", msg.getDate().toString());
    //m->setString("HOST", msg.getHostName());
    m->setString("USER", "b2daq");
    m_producer->send(m);
    delete m;
  } catch (const std::exception& e) {
    LogFile::error(e.what());
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

void LogMQCallback::logget(const std::string& nodename,
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
