#ifndef _Belle2_RunControlCallback_h
#define _Belle2_RunControlCallback_h

#include "daq/slc/apps/runcontrold/RunSetting.h"
#include "daq/slc/apps/runcontrold/RunSummary.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/database/RunNumberInfoTable.h"
#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMNode.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RunControlCallback : public RCCallback {

    typedef std::vector<NSMNode> NSMNodeList;
    typedef NSMNodeList::iterator NSMNodeIterator;
    typedef std::vector<NSMData> NSMDataList;

  public:
    RunControlCallback(const NSMNode& node,
                       const std::string& runtype,
                       const std::string& format,
                       int revision, const int port);
    virtual ~RunControlCallback() throw() {}

  public:
    virtual void init() throw();
    virtual void timeout() throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool log() throw();
    NSMData& getData() { return m_data; }
    void setCallback(RCCallback* callback) throw() {
      m_callback = callback;
    }

  public:
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw() { return distribute(getMessage()); }
    virtual bool pause() throw();
    virtual bool resume() throw() { return distribute(getMessage()); }
    virtual bool abort() throw();
    virtual bool trigft() throw() { return true; }
    virtual bool stateCheck() throw() { return replyOK(); }
    virtual bool exclude() throw();
    virtual bool include() throw();

  public:
    LogFile::Priority getPriorityToDB() const { return m_priority_db; }
    LogFile::Priority getPriorityToLocal() const { return m_priority_local; }
    LogFile::Priority getPriorityToGlobal() const { return m_priority_global; }
    void setPriorityToDB(LogFile::Priority pri) { m_priority_db = pri; }
    void setPriorityToLocal(LogFile::Priority pri) { m_priority_local = pri; }
    void setPriorityToGlobal(LogFile::Priority pri) { m_priority_global = pri; }
    void setExcludedNodes(const StringList& excluded) { m_excluded_v = excluded; }

  private:
    void update() throw();
    bool distribute(NSMMessage& msg) throw();
    bool distribute_r(NSMMessage& msg) throw();
    void postRun() throw();
    ConfigObjectList::iterator findConfig(const std::string& nodename, bool& finded) throw();
    NSMNodeIterator findNode(const std::string& nodename) throw();
    bool synchronize(NSMNode& node) throw();
    void logging(const DAQLogMessage& log, bool recoreded = false);
    bool sendState(const NSMNode& node) throw();
    bool replyOK() throw();

  private:
    RunSetting m_setting;
    RCCallback* m_callback;
    NSMNodeList m_node_v;
    NSMDataList m_data_v;
    RunNumberInfo m_info;
    NSMData m_data;
    std::string m_runtype_default;
    int m_port;
    LogFile::Priority m_priority_db;
    LogFile::Priority m_priority_local;
    LogFile::Priority m_priority_global;
    StringList m_excluded_v;

  private:
    class ConfigProvider {

    public:
      static const int PORT = 20020;

    public:
      ConfigProvider(RunControlCallback* callback,
                     const std::string hostname, int port)
        : m_callback(callback), m_hostname(hostname), m_port(port) {
      }


    public:
      void run();

    private:
      RunControlCallback* m_callback;
      std::string m_hostname;
      int m_port;
    };

  };

}

#endif

