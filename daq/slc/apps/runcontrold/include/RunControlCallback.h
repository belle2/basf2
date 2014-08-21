#ifndef _Belle2_RunControlCallback_h
#define _Belle2_RunControlCallback_h

#include "daq/slc/apps/runcontrold/RunSetting.h"
#include "daq/slc/apps/runcontrold/RunSummary.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/database/RunNumberInfoTable.h"

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
                       const int port);
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
    void addCallback(RCCallback* callback) throw() {
      m_callbacks.push_back(callback);
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
    virtual bool stateCheck() throw() { return true; }

  protected:
    void prepareRun(NSMMessage& msg) throw();
    void postRun(NSMMessage& msg) throw();
    NSMNodeIterator find(const std::string& nodename) throw();
    bool synchronize(NSMNode& node) throw();
    virtual bool isManual() { return true; }

  protected:
    RunSetting m_setting;
    NSMNodeList m_node_v;
    NSMDataList m_data_v;
    NSMMessage m_msg_tmp;
    RunNumberInfo m_info;
    NSMData m_data;
    std::vector<RCCallback*> m_callbacks;
    std::string m_runtype_default;
    int m_port;

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

