#ifndef _Belle2_NsmbridgeCallback_h
#define _Belle2_NsmbridgeCallback_h

#include "daq/slc/nsm/NSMCallback.h"
#include "daq/slc/nsm/NSMVar.h"

#include <map>

namespace Belle2 {

  class NsmbridgeCallback : public NSMCallback {

  public:
    NsmbridgeCallback(const std::string& nodename, int timout = 5);
    virtual ~NsmbridgeCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void vget(const std::string& nodename,
                      const std::string& vname) throw();
    virtual void vset(NSMCommunicator& com,
                      const NSMVar& var) throw();
    virtual void vreply(NSMCommunicator& com,
                        const std::string& name, bool ret) throw();
    void setCallback(NSMCallback* callback)
    {
      m_callback = callback;
    }

  private:
    typedef std::map<std::string, long long> NodeVlist;
    std::map<std::string, NodeVlist> m_vlists;
    std::map<std::string, NSMVar> m_vars;
    NSMCallback* m_callback;

  };

}

#endif
