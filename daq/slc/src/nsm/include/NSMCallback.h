#ifndef _Belle2_NSMCallback_hh
#define _Belle2_NSMCallback_hh

#include "nsm/NSMHandlerException.h"
#include "nsm/NSMMessage.h"

#include "base/NSMNode.h"
#include "base/Command.h"

#include <string>
#include <vector>

namespace Belle2 {

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
    NSMCallback(NSMNode* node) throw();
    virtual ~NSMCallback() throw() {}

  public:
    virtual bool ok() { return true; }
    virtual bool error() { return true; }
    virtual void selfCheck() throw(NSMHandlerException) {}

  public:
    NSMMessage& getMessage();
    NSMCommunicator* getCommunicator() { return _comm; }
    NSMNode* getNode() { return _node; }
    void setCommunicator(NSMCommunicator* comm) { _comm = comm; }
    int getExpNumber();
    int getRunNumber();

  protected:
    void setReply(const std::string& reply) { _reply = reply; }
    void add(const Command& cmd) {
      NSMRequestId req = { -1, cmd};
      _req_v.push_back(req);
    }
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);

  protected:
    std::string _reply;
    NSMNode* _node;

  private:
    NSMRequestList& getRequestList() throw() { return _req_v; }

  private:
    NSMCommunicator* _comm;
    NSMRequestList _req_v;


  };

};

#endif
