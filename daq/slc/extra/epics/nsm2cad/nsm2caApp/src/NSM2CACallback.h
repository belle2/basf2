#ifndef _Belle2_NSM2CACallback_h
#define _Belle2_NSM2CACallback_h

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <dbScan.h>

namespace Belle2 {

  class NSM2CA;

  class NSM2CACallback : public NSMCallback {

  public:
    static NSM2CACallback& get();

  private:
    static NSM2CACallback* g_callback;

  public:
    NSM2CACallback() : NSMCallback(1) {}
    virtual ~NSM2CACallback() throw() {}

  public:
    virtual void init(NSMCommunicator&) throw();
    virtual void timeout(NSMCommunicator&) throw();
    using NSMCallback::get;
    virtual void vreply(NSMCommunicator& com, const std::string& vname, bool success) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();
    virtual void error(const char* nodename, const char* data) throw();
    virtual void ok(const char* nodename, const char* data) throw();

    void setCA(NSM2CA* nsm2ca) { m_nsm2ca = nsm2ca; }

  private:
    NSM2CA* m_nsm2ca;

  };

}

#endif
