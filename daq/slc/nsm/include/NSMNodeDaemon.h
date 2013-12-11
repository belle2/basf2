#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "daq/slc/nsm/NSMCallback.h"
#include "daq/slc/nsm/NSMData.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/NSMNode.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMCallback* callback,
                  const std::string host = "", int port = -1,
                  NSMData* rdata = NULL, NSMData* wdata = NULL);
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();
    void init() throw(NSMHandlerException);
    void setWData(NSMData* data) { _wdata = data; }
    void setRData(NSMData* data) { _rdata = data; }

  private:
    NSMCallback* _callback;
    NSMData* _rdata;
    NSMData* _wdata;
    std::string _host;
    int _port;
    NSMCommunicator* _nsm_comm;

  };

}

#endif
