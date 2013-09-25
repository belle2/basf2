#ifndef _B2DAQ_NSMCallback_hh
#define _B2DAQ_NSMCallback_hh

#include "NSMHandlerException.hh"

#include <node/Command.hh>

#include <string>
#include <vector>

namespace B2DAQ {

  class NSMCommunicator;
  class NSMMessage;

  class NSMCallback {

    friend class NSMCommunicator;

  private:
    struct NSMRequestId {
      int id;
      Command cmd;
    };

  private:
    typedef std::vector<NSMRequestId> NSMRequestList;

  public:
    NSMCallback() throw();
    virtual ~NSMCallback() throw() {}

  public:
    virtual bool ok() { return true; }
    virtual bool error() { return true; }
    virtual void selfCheck() throw(NSMHandlerException) {}

  public:
    NSMCommunicator* getCommunicator() { return _comm; }
    void setCommunicator(NSMCommunicator* comm) { _comm = comm; }

  protected:
    void setReply(const std::string& reply) { _reply = reply; }
    void add(const Command& cmd) {
      NSMRequestId req = {-1, cmd};
      _req_v.push_back(req);
    }
    //Command findCommand(NSMMessage& msg) throw(NSMHandlerException);
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);
    
  protected:
    std::string _reply;

  private:
    NSMRequestList& getRequestList() throw() { return _req_v; }

  private:
    NSMCommunicator* _comm;
    NSMRequestList _req_v;


  };

};

#endif
