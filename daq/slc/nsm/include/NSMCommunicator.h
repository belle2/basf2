#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "daq/slc/nsm/NSMHandlerException.h"
#include "daq/slc/nsm/NSMNotConnectedException.h"
#include "daq/slc/nsm/NSMMessage.h"
#include "daq/slc/nsm/NSMNode.h"

#include <daq/slc/system/Mutex.h>

#include <daq/slc/database/DAQLogMessage.h>

#include <daq/slc/base/TimeoutException.h>
#include <daq/slc/base/Serializable.h>
#include <daq/slc/base/ERRORNo.h>

extern "C" {
#include <nsm2/nsm2.h>
}

#include <math.h>

#include <queue>
#include <vector>
#include <map>

namespace Belle2 {

  class NSMCallback;
  class NSMMessage;

  class NSMCommunicator {

    typedef std::vector<NSMCommunicator*> NSMCommunicatorList;

    friend class NSMData;

  public:
    static NSMCommunicator& select(double sec) throw(IOException);
    static NSMCommunicator& connected(const std::string& node)
    throw(NSMNotConnectedException);
    static NSMCommunicatorList& get() throw() { return g_comm; }
    static bool send(const NSMMessage& msg) throw(NSMHandlerException);

  private:
    static NSMCommunicatorList g_comm;
    static Mutex g_mutex;
    static Mutex g_mutex_select;

  public:
    NSMCommunicator(const std::string& host = "", int port = -1) throw();
    NSMCommunicator(NSMcontext* nsmc) throw();
    ~NSMCommunicator() throw() {}

  public:
    void init(const NSMNode& node, const std::string& host, int port)
    throw(NSMHandlerException);
    void setCallback(NSMCallback* callback) throw(NSMHandlerException);
    void callContext() throw(NSMHandlerException);

  public:
    int getId() const throw() { return m_id; }
    void setId(int id) throw() { m_id = id; }
    NSMMessage& getMessage() throw() { return m_message; }
    const NSMMessage& getMessage() const throw() { return m_message; }
    void setMessage(const NSMMessage& msg) throw();// { m_message = msg; }
    const NSMNode& getNode() const throw() { return m_node; }
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);
    const std::string getNodeHost(const std::string& nodename) throw();
    const std::string getNodeHost() throw();
    const std::string& getHostName() throw() { return m_host; }
    int getPort() throw() { return m_port; }
    NSMCallback& getCallback() throw(std::out_of_range);
    bool isConnected(const std::string& node) throw();
    const std::string getNodeNameById(int id) throw(NSMHandlerException);
    void pushQueue(const NSMMessage& msg) { m_msg_q.push(msg); }
    bool hasQueue() const { return !m_msg_q.empty(); }
    NSMMessage popQueue();

  private:
    NSMcontext* getContext() { return m_nsmc; }

  private:
    bool sendRequest(const NSMMessage& msg) throw(NSMHandlerException);

  private:
    int m_id;
    NSMcontext* m_nsmc;
    NSMCallback* m_callback;
    NSMMessage m_message;
    std::string m_host;
    int m_port;
    NSMNode m_node;
    std::queue<NSMMessage> m_msg_q;

  };

  typedef std::vector<NSMCommunicator*> NSMCommunicatorList;

};

#endif
