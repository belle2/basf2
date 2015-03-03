#ifndef _Belle2_NSMCallback_hh
#define _Belle2_NSMCallback_hh

#include "daq/slc/nsm/AbstractNSMCallback.h"
#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMCommand.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class NSMCommunicator;
  class NSMMessage;

  typedef std::map<std::string, NSMNode> NSMNodeMap;

  class NSMCallback : public AbstractNSMCallback {

    friend class NSMCommunicator;
    friend class NSMNodeDaemon;

  public:
    NSMCallback(int timeout = 5) throw();
    virtual ~NSMCallback() throw() {}

  public:
    NSMNode& getNode() throw() { return m_node; }
    const NSMNode& getNode() const throw() { return m_node; }
    void setNode(const NSMNode& node) throw() { m_node = node; }

  public:
    virtual void init(NSMCommunicator&) throw() {}
    virtual void term() throw() {}
    virtual void timeout(NSMCommunicator&) throw() {}
    virtual bool perform(NSMCommunicator& com) throw();
    virtual void ok(const char* /*node*/, const char* /*data*/) throw() {}
    virtual void error(const char* /*node*/, const char* /*data*/) throw() {}
    virtual void log(const DAQLogMessage&) throw() {}
    virtual void vget(NSMCommunicator& com, const std::string& vname) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void vlistget(NSMCommunicator& com) throw();
    virtual void vlistset(NSMCommunicator& com) throw();
    virtual void vreply(NSMCommunicator&, const std::string&, bool) throw() {}

  public:
    bool reply(const NSMMessage& msg) throw(NSMHandlerException);

  protected:
    virtual void notify(const NSMVar& var) throw();
    void reg(const NSMCommand& cmd) throw() { m_cmd_v.push_back(cmd); }
    void addNode(const NSMNode& node);
    const NSMNodeMap& getNodes() { return m_nodes; }

  private:
    typedef std::vector<NSMCommand> NSMCommandList;

  private:
    NSMCommandList& getCommandList() throw() { return m_cmd_v; }

  private:
    NSMNode m_node;
    NSMCommandList m_cmd_v;
    NSMNodeMap m_nodes;

  };

};

#endif
