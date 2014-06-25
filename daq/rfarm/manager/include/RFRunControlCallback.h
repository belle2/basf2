#ifndef _Belle2_RFRunControlCallback_hh
#define _Belle2_RFRunControlCallback_hh

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class RFMaster;
  class RFMasterCallback;

  class RFRunControlCallback : public RCCallback {

  public:
    RFRunControlCallback(const NSMNode& node, RFMaster* master,
                         RFMasterCallback* callback);
    virtual ~RFRunControlCallback() throw();

  public:
    virtual void init() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  public:
    NSMData& getData() throw() { return m_data; }

  private:
    RFMaster* m_master;
    RFMasterCallback* m_callback;
    NSMData m_data;

  };

}

#endif
