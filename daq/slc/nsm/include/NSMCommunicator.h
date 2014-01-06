#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMMessage.h"

#include "daq/slc/base/NSMNode.h"
#include "daq/slc/base/Command.h"
#include "daq/slc/base/SystemLog.h"

extern "C" {
#include "nsm2/nsm2.h"
}

#include <vector>
#include <map>

namespace Belle2 {

  class NSMCallback;
  class NSMMessage;

  class NSMCommunicator {

  public:
    static NSMCommunicator* select(int timeout) throw(NSMHandlerException);

  private:
    static std::vector<NSMCommunicator*> __com_v;

  public:
    NSMCommunicator(NSMNode* node = NULL, const std::string& host = "",
                    int port = -1, const std::string& config_name = "slowcontrol") throw();
    ~NSMCommunicator() throw() {}

  public:
    void init(const std::string& host = "", int port = -1) throw(NSMHandlerException);
    bool wait(int sec) throw(NSMHandlerException);
    void sendRequest(const NSMNode* node, const Command& cmd,
                     int npar = 0, int* pars = NULL,
                     int len = 0, const char* datap = NULL) throw(NSMHandlerException);
    void sendRequest(const NSMNode* node, const Command& cmd,
                     int npar, int* pars,
                     const std::string& message) throw(NSMHandlerException);
    void sendRequest(const NSMNode* node, const Command& cmd,
                     NSMMessage& message) throw(NSMHandlerException);
    void sendRequest(const NSMNode* node, const Command& cmd,
                     const std::string& message) throw(NSMHandlerException);
    void replyOK(const NSMNode* node, const std::string& message = "")
    throw(NSMHandlerException);
    void replyError(const std::string& message = "") throw(NSMHandlerException);
    void sendLog(const SystemLog& log) throw(NSMHandlerException);
    void sendError(const std::string& message) throw(NSMHandlerException);
    void sendState() throw(NSMHandlerException);

  public:
    NSMcontext* getContext() throw() { return _nsmc; }
    int getId() const throw() { return _id; }
    bool isOnline() const throw() { return (_id >= 0); }
    NSMNode* getNode() throw() { return _node; }
    NSMCallback* getCallback() throw() { return _callback; }
    NSMMessage& getMessage() throw() { return _message; }
    void setMessage(NSMMessage& msg) throw() { _message = msg; }
    void setId(int id) throw() { _id = id; }
    void setNode(NSMNode* node) throw() { _node = node; }
    void setCallback(NSMCallback* callback) throw() { _callback = callback; }
    bool performCallback() throw(NSMHandlerException);
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);
    void setContext(NSMcontext* nsmc) throw(NSMHandlerException);

  private:
    std::string _config_name;
    NSMNode* _node;
    NSMCallback* _callback;
    int _id;
    NSMMessage _message;
    NSMcontext* _nsmc;
    std::string _host;
    int _port;
    NSMNode* _logger_node;
    NSMNode* _rc_node;

  };

};

#endif
