#ifndef _Belle2_LogMQCallback_h
#define _Belle2_LogMQCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <map>
#include <string>
#include <vector>

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace cms;

namespace Belle2 {

  class DBInterface;

  class LogMQCallback : public NSMCallback {

    typedef std::map<std::string, NSMNode> NSMNodeList;
    typedef std::map<std::string, LogFile::Priority> PriorityList;

  public:
    LogMQCallback(const std::string& nodename,
                  DBInterface& db, int timout = 5);
    virtual ~LogMQCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void logset(const DAQLogMessage& msg) throw();
    virtual void logget(const std::string& nodename,
                        LogFile::Priority pri) throw();

  public:
    void setDBTable(const std::string& logtable)
    {
      m_logtable = logtable;
    }

  private:
    DBInterface& m_db;
    std::string m_logtable;
    NSMNodeList m_nodes;
    PriorityList m_pris;
    std::vector<DAQLogMessage> m_msgs;

    ActiveMQConnectionFactory* m_connectionFactory;
    cms::Connection* m_connection;
    Session* m_session;
    Topic* m_destination;
    MessageProducer* m_producer;

  };

}

#endif
