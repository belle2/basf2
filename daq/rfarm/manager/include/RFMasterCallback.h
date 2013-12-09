#ifndef _Belle2_RFMasterCallback_hh
#define _Belle2_RFMasterCallback_hh

#include <daq/slc/nsm/NSMCallback.h>
#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class RFMaster;

  class RFMasterCallback : public NSMCallback {

  public:
    RFMasterCallback(NSMNode* node, NSMData* data, RFMaster* master);
    virtual ~RFMasterCallback() throw();

  public:
    virtual bool perform(const Command& command, NSMMessage& msg)
    throw(NSMHandlerException);
    virtual void init() throw();
    virtual bool ok() throw();
    virtual bool error() throw();
    virtual bool configure() throw();
    virtual bool unconfigure() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool pause() throw();
    virtual bool resume() throw();
    virtual bool restart() throw();
    virtual bool status() throw();

  private:
    NSMData* _data;
    RFMaster* _master;

  };

}

#endif
