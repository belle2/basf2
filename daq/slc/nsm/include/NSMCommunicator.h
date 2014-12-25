#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"

#include <daq/slc/system/Mutex.h>

#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/base/Serializable.h>
#include <daq/slc/base/ERRORNo.h>

extern "C" {
#include <nsm2/nsm2.h>
}

#include <vector>
#include <map>

namespace Belle2 {

  class NSMCallback;
  class NSMMessage;

  typedef std::map<std::string, NSMNode> NSMNodeList;

  class NSMCommunicator {

  public:
    static int select(int sec, NSMCommunicator** com,
                      int* flags, int ncoms)
    throw(NSMHandlerException);

  public:
    NSMCommunicator(const std::string& host = "", int port = -1) throw();
    ~NSMCommunicator() throw() {}

  public:
    void init(const NSMNode& node,
              const std::string& host, int port)
    throw(NSMHandlerException);
    bool wait(int sec) throw(NSMHandlerException);
    void setCallback(NSMCallback* callback) throw(NSMHandlerException);
    void sendRequest(const NSMMessage& msg) throw(NSMHandlerException);
    void sendRequest(const std::string& node, const std::string& cmd,
                     int npar, const int* pars,
                     int len, const char* data) throw(NSMHandlerException);
    void sendExclude(const NSMNode& node) throw(NSMHandlerException);
    void sendInclude(const NSMNode& node) throw(NSMHandlerException);
    void replyOK(const NSMNode& node) throw(NSMHandlerException);
    void replyError(int error, const std::string& message = "")
    throw(NSMHandlerException);
    bool sendLog(const DAQLogMessage& log, bool recored = false);
    bool sendLog(const NSMNode& node, const DAQLogMessage& log,
                 bool recored = false);
    bool sendError(const std::string& message);
    bool sendError(int error, //const std::string& nodename,
                   const std::string& message);
    bool sendFatal(const std::string& message);
    void sendState(const NSMNode& node) throw(NSMHandlerException);
    void readContext(NSMcontext* nsmc) throw();

  public:
    NSMcontext* getContext() throw() { return m_nsmc; }
    int getId() const throw() { return m_id; }
    bool isOnline() const throw() { return (m_id >= 0); }
    NSMMessage& getMessage() throw() { return m_message; }
    const NSMMessage& getMessage() const throw() { return m_message; }
    void setMessage(const NSMMessage& msg) throw() { m_message = msg; }
    void setId(int id) throw() { m_id = id; }
    const NSMNode& getNode() const throw() { return m_node; }
    const NSMNodeList& getMasters() const throw() { return m_masters; }
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);
    void setContext(NSMcontext* nsmc) throw(NSMHandlerException);
    bool isConnected(const NSMNode& node) throw() {
      return isConnected(node.getName());
    }
    bool isConnected(const std::string& node) throw();
    const std::string getNodeHost(const std::string& nodename) throw();
    const std::string getNodeHost() throw();
    const std::string& getHostName() throw() { return m_host; }
    int getPort() throw() { return m_port; }

  private:
    int m_id;
    NSMMessage m_message;
    NSMcontext* m_nsmc;
    std::string m_host;
    int m_port;
    NSMNodeList m_masters;
    NSMNode m_node;
    Mutex m_mutex;

  };

};

#endif
