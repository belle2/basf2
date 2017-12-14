#ifndef _Belle2_NSMCallback_hh
#define _Belle2_NSMCallback_hh

#include "daq/slc/nsm/AbstractNSMCallback.h"
#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMCommand.h"
#include "daq/slc/nsm/NSMData.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NSMCommunicator;
  class NSMMessage;

  typedef std::map<std::string, NSMNode> NSMNodeMap;
  typedef std::map<std::string, NSMData> NSMDataMap;

  class NSMCallback : public AbstractNSMCallback {

    friend class NSMCommunicator;
    friend class NSMNodeDaemon;

  public:
    NSMCallback(int timeout = 5) throw();
    virtual ~NSMCallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw() {}
    virtual void term() throw() {}
    virtual void timeout(NSMCommunicator&) throw() {}
    virtual bool perform(NSMCommunicator& com) throw();
    virtual void ok(const char* /*node*/, const char* /*data*/) throw() {}
    virtual void fatal(const char* /*node*/, const char* /*data*/) throw() {}
    virtual void error(const char* /*node*/, const char* /*data*/) throw() {}
    virtual void logset(const DAQLogMessage&) throw() {}
    virtual void logget(const std::string& /*nodename*/,
                        LogFile::Priority /*priorit*/) throw() {}
    virtual void vget(const std::string& nodename,
                      const std::string& vname) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void vlistget(NSMCommunicator& com) throw();
    virtual void vlistset(NSMCommunicator& com) throw();
    virtual void vreply(NSMCommunicator&, const std::string&, bool) throw() {}
    virtual void nsmdataset(NSMCommunicator& com, NSMData& data) throw();
    virtual void nsmdataget(NSMCommunicator& com) throw();

  public:
    void reply(const NSMMessage& msg) throw(NSMHandlerException);
    void log(LogFile::Priority pri, const char* format, ...);
    void log(LogFile::Priority pri, const std::string& msg);

  public:
    NSMDataMap& getDataMap() throw() { return m_datas; }
    NSMData& getData(const std::string& name) throw(std::out_of_range);
    NSMData& getData() throw() { return m_data; }
    void openData(const std::string& name, const std::string& format,
                  int revision = -1) throw();
    void allocData(const std::string& name, const std::string& format,
                   int revision) throw();

  protected:
    virtual void notify(const NSMVar& var) throw();
    void reg(const NSMCommand& cmd) throw() { m_cmd_v.push_back(cmd); }
    void addNode(const NSMNode& node);
    const NSMNodeMap& getNodes() { return m_nodes; }
    void setLogNode(const NSMNode& node) { m_lognode = node; }
    const NSMNode& getLogNode() { return m_lognode; }

  private:
    typedef std::vector<NSMCommand> NSMCommandList;

  private:
    NSMCommandList& getCommandList() throw() { return m_cmd_v; }
    void alloc_open(NSMCommunicator& com) throw();

  private:
    NSMCommandList m_cmd_v;
    NSMNodeMap m_nodes;
    NSMData m_data;
    NSMDataMap m_datas;
    NSMNode m_lognode;

  };

  inline NSMData& NSMCallback::getData(const std::string& name) throw(std::out_of_range)
  {
    if (m_datas.find(name) != m_datas.end()) {
      return m_datas[name];
    }
    throw (std::out_of_range("nodata registered : " + name));
  }

  inline void NSMCallback::openData(const std::string& name,
                                    const std::string& format,
                                    int revision) throw()
  {
    if (m_datas.find(name) == m_datas.end()) {
      m_datas.insert(NSMDataMap::value_type(name, NSMData(name, format, revision)));
    }
  }

  inline void NSMCallback::allocData(const std::string& name,
                                     const std::string& format,
                                     int revision) throw()
  {
    m_data = NSMData(name, format, revision);
  }

};

#endif
