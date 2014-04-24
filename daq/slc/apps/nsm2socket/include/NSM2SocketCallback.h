#ifndef _Belle2_NSM2SocketCallback_hh
#define _Belle2_NSM2SocketCallback_hh

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  typedef std::vector<NSMData> NSMDataList;

  class NSM2SocketBridge;

  class NSM2SocketCallback : public NSMCallback {

  public:
    NSM2SocketCallback(const NSMNode& node, int interval, ConfigFile& file) throw();
    virtual ~NSM2SocketCallback() throw() {}

  public:
    virtual void init() throw();
    virtual void timeout() throw();

  public:
    NSMDataList& getDataList() { return m_data_v; }
    void setBridge(NSM2SocketBridge* bridge) { m_bridge = bridge; }
    bool sendRequest(NSMMessage& msg) throw();

  public:
    virtual bool perform(NSMMessage& msg) throw(NSMHandlerException);

  protected:
    NSMDataList m_data_v;
    NSM2SocketBridge* m_bridge;
    Mutex m_mutex;

  };

};

#endif
