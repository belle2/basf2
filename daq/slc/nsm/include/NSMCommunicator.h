#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"

#include <daq/slc/base/SystemLog.h>
#include <daq/slc/base/Serializable.h>

extern "C" {
#include <nsm2/nsm2.h>
}

#include <vector>
#include <map>

namespace Belle2 {

  class NSMCallback;
  class NSMMessage;

  class NSMCommunicator {

  public:
    NSMCommunicator(const std::string& host = "",
                    int port = -1) throw();
    ~NSMCommunicator() throw() {}

  public:
    void init(const NSMNode& node,
              const std::string& host = "", int port = -1)
    throw(NSMHandlerException);
    bool wait(int sec) throw(NSMHandlerException);
    void setCallback(NSMCallback* callback) throw(NSMHandlerException);
    void sendRequest(const NSMMessage& msg) throw(NSMHandlerException);
    void sendRequest(const std::string& node, const std::string& cmd,
                     int npar, const int* pars,
                     int len, const char* data) throw(NSMHandlerException);
    void replyOK(const NSMNode& node) throw(NSMHandlerException);
    void replyError(const std::string& message = "") throw(NSMHandlerException);
    bool sendLog(const SystemLog& log);
    bool sendLog(const NSMNode& node, const SystemLog& log);
    bool sendError(const std::string& message);
    bool sendFatal(const std::string& message);
    void sendState(const NSMNode& node) throw(NSMHandlerException);

  public:
    NSMcontext* getContext() throw() { return _nsmc; }
    int getId() const throw() { return _id; }
    bool isOnline() const throw() { return (_id >= 0); }
    NSMMessage& getMessage() throw() { return _message; }
    const NSMMessage& getMessage() const throw() { return _message; }
    void setMessage(const NSMMessage& msg) throw() { _message = msg; }
    void setId(int id) throw() { _id = id; }
    const NSMNode& getNode() const throw() { return _node; }
    const NSMNode& getMaster() const throw() { return _master_node; }
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);
    void setContext(NSMcontext* nsmc) throw(NSMHandlerException);
    bool isConnected(const NSMNode& node) throw();

  private:
    int _id;
    NSMMessage _message;
    NSMcontext* _nsmc;
    std::string _host;
    int _port;
    NSMNode _master_node;
    NSMNode _node;

  };

};

#endif
