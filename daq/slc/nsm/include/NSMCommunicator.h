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
    static NSMCommunicator& select(double sec);
    static NSMCommunicator& connected(const std::string& node);
    static NSMCommunicatorList& get() { return g_comm; }
    static bool send(const NSMMessage& msg);

  private:
    static NSMCommunicatorList g_comm;
    static Mutex g_mutex;
    static Mutex g_mutex_select;

  public:
    NSMCommunicator(const std::string& host = "", int port = -1);
    NSMCommunicator(NSMcontext* nsmc);
    ~NSMCommunicator() {}

  public:
    void init(const NSMNode& node, const std::string& host, int port)
    ;
    void setCallback(NSMCallback* callback);
    void callContext();

  public:
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    NSMMessage& getMessage() { return m_message; }
    const NSMMessage& getMessage() const { return m_message; }
    void setMessage(const NSMMessage& msg);
    const NSMNode& getNode() const { return m_node; }
    int getNodeIdByName(const std::string& name);
    int getNodePidByName(const std::string& name);
    const std::string getNodeHost(const std::string& nodename);
    const std::string getNodeHost();
    const std::string& getHostName() { return m_host; }
    int getPort() { return m_port; }
    NSMCallback& getCallback();
    bool isConnected(const std::string& node);
    const std::string getNodeNameById(int id);
    void pushQueue(const NSMMessage& msg) { m_msg_q.push(msg); }
    bool hasQueue() const { return !m_msg_q.empty(); }
    NSMMessage popQueue();

  private:
    NSMcontext* getContext() { return m_nsmc; }

  private:
    bool sendRequest(const NSMMessage& msg);

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
