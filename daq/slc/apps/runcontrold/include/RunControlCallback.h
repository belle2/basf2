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
    typedef std::map<std::string, NSMNode> NSMNodeMap;
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
    virtual void update() throw();
    virtual bool send(const NSMMessage msg) throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool log() throw();
    NSMData& getData() { return m_data; }
    void setCallback(RCCallback* callback) throw() {
      m_callback = callback;
    }

  public:
    virtual bool load() throw() { return send(getMessage()); }
    virtual bool start() throw() { return send(getMessage()); }
    virtual bool stop() throw() { return send(getMessage()); }
    virtual bool recover() throw() { return send(getMessage()); }
    virtual bool pause() throw();
    virtual bool resume() throw() { return send(getMessage()); }
    virtual bool abort() throw() { return send(getMessage()); }
    virtual bool trigft() throw() { return send(getMessage()); }
    virtual bool stateCheck() throw();
    virtual bool exclude() throw();
    virtual bool include() throw();

  public:
    LogFile::Priority getPriorityToDB() const { return m_priority_db; }
    LogFile::Priority getPriorityToLocal() const { return m_priority_local; }
    LogFile::Priority getPriorityToGlobal() const { return m_priority_global; }
    void setPriorityToDB(LogFile::Priority pri) { m_priority_db = pri; }
    void setPriorityToLocal(LogFile::Priority pri) { m_priority_local = pri; }
    void setPriorityToGlobal(LogFile::Priority pri) { m_priority_global = pri; }

  protected:
    void prepareRun(NSMMessage& msg) throw();
    void postRun(NSMMessage& msg) throw();
    NSMNodeIterator find(const std::string& nodename) throw();
    bool synchronize(NSMNode& node) throw();
    virtual bool isManual() { return true; }
    void logging(const DAQLogMessage& log, bool recoreded = false);

  protected:
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

