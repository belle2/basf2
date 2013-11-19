#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "nsm/NSMCallback.h"
#include "nsm/NSMData.h"

#include "base/NSMNode.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMCallback* callback = NULL,
                  const std::string host = "", int port = -1,
                  NSMData* rdata = NULL, NSMData* wdata = NULL)
      : _callback(callback), _rdata(rdata), _wdata(wdata),
        _host(host), _port(port) {}
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();

  private:
    NSMCallback* _callback;
    NSMData* _rdata;
    NSMData* _wdata;
    std::string _host;
    int _port;
  };

}

#endif
