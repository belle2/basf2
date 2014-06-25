#ifndef _Belle2_RunControlCallback_h
#define _Belle2_RunControlCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/database/RunNumberInfoTable.h"
#include "daq/slc/runcontrol/RunSetting.h"
#include "daq/slc/runcontrol/RunSummary.h"

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
    RunControlCallback(const NSMNode& node);
    virtual ~RunControlCallback() throw() {}

  public:
    virtual void init() throw();
    virtual void timeout() throw();
    virtual void update() throw();
    virtual bool send(const NSMMessage msg) throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool log() throw();
    virtual void addCallback(RCCallback* callback) throw() {
      m_callbacks.push_back(callback);
    }
    NSMData& getData() { return m_data; }

  public:
    virtual bool load() throw() { return send(getMessage()); }
    virtual bool start() throw() { return send(getMessage()); }
    virtual bool stop() throw() { return send(getMessage()); }
    virtual bool recover() throw() { return send(getMessage()); }
    virtual bool resume() throw() { return send(getMessage()); }
    virtual bool pause() throw() { return send(getMessage()); }
    virtual bool abort() throw() { return send(getMessage()); }
    virtual bool trigft() throw() { return send(getMessage()); }
    virtual bool stateCheck() throw() { return true; }

  protected:
    void prepareRun(NSMMessage& msg) throw();
    void postRun(NSMMessage& msg) throw();
    NSMNodeIterator find(const std::string& nodename) throw();
    NSMNodeIterator synchronize(NSMNode& node) throw();
    virtual bool isManual() { return true; }

  protected:
    RunSetting m_setting;
    NSMNodeList m_node_v;
    NSMDataList m_data_v;
    NSMMessage m_msg_tmp;
    RunNumberInfo m_info;
    NSMData m_data;
    std::vector<RCCallback*> m_callbacks;

  };

}

#endif

