#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "NSMHandlerException.h"
#include "NSMMessage.h"

#include "base/NSMNode.h"
#include "base/Command.h"

extern "C" {
#include <nsm2.h>
}

#include <vector>

namespace Belle2 {

  class NSMCallback;

  class NSMCommunicator {

  public:
    NSMCommunicator(NSMNode* node = NULL) throw();
    virtual ~NSMCommunicator() throw() {}

  public:
    virtual void init(bool usesig = false) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     int npar = 0, unsigned int* pars = NULL,
                     int len = 0, const char* datap = NULL) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     int npar, unsigned int* pars,
                     const std::string& message) throw(NSMHandlerException) {
      sendRequest(node, cmd, npar, pars, message.size(),
                  (message.size() == 0) ? NULL : message.c_str());
    }
    void sendRequest(NSMNode* node, const Command& cmd,
                     const std::string& message) throw(NSMHandlerException) {
      sendRequest(node, cmd, 0, NULL, message.size(), message.c_str());
    }
    void replyOK(NSMNode* node, const std::string& message = "") throw(NSMHandlerException);
    void replyError(const std::string& message = "") throw(NSMHandlerException);
    bool wait(int sec) throw(NSMHandlerException);

  public:
    int getId() const throw() { return _id; }
    bool isOnline() const throw() { return (_id >= 0); }
    NSMNode* getNode() throw() { return _node; }
    NSMCallback* getCallback() throw() { return _callback; }
    NSMMessage& getMessage() throw() { return _message; }
    void setId(int id) throw() { _id = id; }
    void setNode(NSMNode* node) throw() { _node = node; }
    void setCallback(NSMCallback* callback) throw() { _callback = callback; }
    bool performCallback() throw(NSMHandlerException);
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);

  private:
    int _id;
    NSMNode* _node;
    NSMCallback* _callback;
    NSMMessage _message;
    NSMcontext* _nsmc;

  };

};

#endif
