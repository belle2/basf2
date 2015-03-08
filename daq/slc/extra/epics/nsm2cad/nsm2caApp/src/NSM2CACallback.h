#ifndef _Belle2_NSM2CACallback_h
#define _Belle2_NSM2CACallback_h

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <dbScan.h>

namespace Belle2 {

  class NSMVHandler2CA : public NSMVHandler {

  public:
    NSMVHandler2CA(const std::string& node,
                   const std::string& name, IOSCANPVT& pvt)
      : NSMVHandler(node, name, true, true), m_pvt(pvt) {}
    virtual ~NSMVHandler2CA() throw() {}
    virtual bool handleSet(const NSMVar& var);

  private:
    IOSCANPVT& m_pvt;

  };

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
    virtual void error(const char* nodename, const char* data) throw();
    virtual void ok(const char* nodename, const char* data) throw();

  };

}

#endif
