#ifndef _Belle2_NSM2SocketCallback_hh
#define _Belle2_NSM2SocketCallback_hh

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPSocket.h>

#include <daq/slc/base/ConfigFile.h>

#include <map>

namespace Belle2 {

  class NSM2SocketBridge;

  class NSM2SocketCallback : public NSMCallback {

    typedef std::map<std::string, NSMData> NSMDataMap;
    typedef std::map<std::string, NSMNode> NSMNodeMap;

  public:
    NSM2SocketCallback(const NSMNode& node) throw();
    virtual ~NSM2SocketCallback() throw() {}

  public:
    NSMData& getData(const std::string& name,
                     const std::string& format,
                     int revision) throw(NSMHandlerException);
    void setBridge(NSM2SocketBridge* bridge) { m_bridge = bridge; }
    bool send(const NSMMessage& msg) throw();

  public:
    virtual bool perform(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void logset(const DAQLogMessage& logmsg) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& v) throw();
    void requestLog() throw();

  protected:
    NSM2SocketBridge* m_bridge;
    Mutex m_mutex;
    NSMNodeMap m_nodes;
    NSMDataMap m_datas;

  };

};

#endif
