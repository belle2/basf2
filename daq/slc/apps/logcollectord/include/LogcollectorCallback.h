#ifndef _Belle2_LogcollectorCallback_h
#define _Belle2_LogcollectorCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <map>
#include <string>
#include <vector>

namespace Belle2 {

  class DBInterface;
  typedef std::map<std::string, NSMNode> NSMNodeList;
  typedef std::map<std::string, LogFile::Priority> PriorityList;

  class LogcollectorCallback : public NSMCallback {

  public:
    LogcollectorCallback(const std::string& nodename,
                         DBInterface& db, int timout = 5);
    virtual ~LogcollectorCallback() throw();

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

  };

}

#endif
