#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "daq/slc/nsm/NSMCallback.h"
#include "daq/slc/nsm/NSMData.h"
#include "daq/slc/nsm/NSMCommunicator.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMCallback* callback,
                  const std::string host = "", int port = -1);
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();
    void init() throw(NSMHandlerException);

  private:
    NSMCallback* _callback;
    std::string _host;
    int _port;
    NSMCommunicator* _nsm_comm;

  };

}

#endif
