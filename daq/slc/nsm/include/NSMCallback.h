#ifndef _Belle2_NSMCallback_hh
#define _Belle2_NSMCallback_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"
#include "daq/slc/nsm/NSMCommand.h"

#include <string>
#include <vector>

namespace Belle2 {

  class NSMCommunicator;
  class NSMMessage;

  class NSMCallback {

    friend class NSMCommunicator;

  private:
    typedef std::vector<NSMCommand> NSMRequestList;

  public:
    NSMCallback(const NSMNode& node, int timeout = 5) throw();
    virtual ~NSMCallback() throw() {}

  public:
    virtual void init() throw() {}
    virtual void term() throw() {}
    virtual bool ok() throw() { return true; }
    virtual bool error() throw() { return true; }
    virtual bool fatal() throw() { return true; }
    virtual bool log() throw() { return true; }
    virtual bool state() throw() { return true; }
    virtual void timeout() throw() {}
    bool isReady() const throw();

  public:
    NSMMessage& getMessage();
    void setMessage(NSMMessage& msg);
    NSMCommunicator* getCommunicator() { return _comm; }
    NSMNode& getNode() throw() { return _node; }
    const NSMNode& getNode() const throw() { return _node; }
    void setCommunicator(NSMCommunicator* comm) { _comm = comm; }
    int getTimeout() const { return _timeout; }

  public:
    void setReply(const std::string& reply) throw() { _reply = reply; }
    const std::string& getReply() const throw() { return _reply; }
    void add(const NSMCommand& cmd) throw() { _cmd_v.push_back(cmd); }
    virtual bool perform(const NSMMessage& msg) throw();

  private:
    NSMRequestList& getRequestList() throw() { return _cmd_v; }

  private:
    std::string _reply;
    NSMNode _node;
    NSMCommunicator* _comm;
    NSMRequestList _cmd_v;
    int _timeout;

  };

};

#endif
