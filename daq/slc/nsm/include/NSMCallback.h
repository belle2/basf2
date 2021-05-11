#ifndef _Belle2_NSMCallback_hh
#define _Belle2_NSMCallback_hh

#include "daq/slc/nsm/AbstractNSMCallback.h"
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
    NSMCallback(int timeout = 5);
    virtual ~NSMCallback() {}

  public:
    virtual void init(NSMCommunicator&) {}
    virtual void term() {}
    virtual void timeout(NSMCommunicator&) {}
    virtual bool perform(NSMCommunicator& com);
    virtual void ok(const char* /*node*/, const char* /*data*/) {}
    virtual void fatal(const char* /*node*/, const char* /*data*/) {}
    virtual void error(const char* /*node*/, const char* /*data*/) {}
    virtual void logset(const DAQLogMessage&) {}
    virtual void vget(const std::string& nodename,
                      const std::string& vname);
    virtual void vset(NSMCommunicator& com, const NSMVar& var);
    virtual void vlistget(NSMCommunicator& com);
    virtual void vlistset(NSMCommunicator& com);
    virtual void vreply(NSMCommunicator&, const std::string&, bool) {}

  public:
    void reply(const NSMMessage& msg);
    void log(LogFile::Priority pri, const char* format, ...);
    void log(LogFile::Priority pri, const std::string& msg);
    int reset();

  public:
    NSMDataMap& getDataMap() { return m_datas; }
    NSMData& getData(const std::string& name);
    NSMData& getData() { return m_data; }
    void openData(const std::string& name, const std::string& format,
                  int revision = -1);
    void allocData(const std::string& name, const std::string& format,
                   int revision);
    const std::string& getCategory() const { return m_category; }
    void setCategory(const std::string& category) { m_category = category; }

  protected:
    virtual void notify(const NSMVar& var);
    void reg(const NSMCommand& cmd) { m_cmd_v.push_back(cmd); }
    void addNode(const NSMNode& node);
    const NSMNodeMap& getNodes() { return m_nodes; }
    void setLogNode(const NSMNode& node) { m_lognode = node; }
    const NSMNode& getLogNode() { return m_lognode; }

  private:
    typedef std::vector<NSMCommand> NSMCommandList;

  private:
    int addDefaultHandlers();

    NSMCommandList& getCommandList() { return m_cmd_v; }
    void alloc_open(NSMCommunicator& com);

  private:
    NSMCommandList m_cmd_v;
    NSMNodeMap m_nodes;
    NSMData m_data;
    NSMDataMap m_datas;
    NSMNode m_lognode;
    std::string m_category;

  };

  inline NSMData& NSMCallback::getData(const std::string& name)
  {
    if (m_datas.find(name) != m_datas.end()) {
      return m_datas[name];
    }
    throw (std::out_of_range("nodata registered : " + name));
  }

  inline void NSMCallback::openData(const std::string& name,
                                    const std::string& format,
                                    int revision)
  {
    if (m_datas.find(name) == m_datas.end()) {
      m_datas.insert(NSMDataMap::value_type(name, NSMData(name, format, revision)));
    }
  }

  inline void NSMCallback::allocData(const std::string& name,
                                     const std::string& format,
                                     int revision)
  {
    m_data = NSMData(name, format, revision);
  }

};

#endif
