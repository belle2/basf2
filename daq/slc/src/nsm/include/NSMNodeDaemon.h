#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "nsm/NSMCallback.h"
#include "nsm/NSMData.h"

#include "base/NSMNode.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMNode* node = NULL, NSMCallback* callback = NULL,
                  const std::string host1 = "", int port1 = -1,
                  const std::string host2 = "", int port2 = -1,
                  NSMData* rdata = NULL, NSMData* wdata = NULL)
      : _node(node), _callback(callback), _rdata(rdata), _wdata(wdata),
        _host1(host1), _port1(port1), _host2(host2), _port2(port2) {}
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();

  private:
    NSMNode* _node;
    NSMCallback* _callback;
    NSMData* _rdata;
    NSMData* _wdata;
    std::string _host1;
    int _port1;
    std::string _host2;
    int _port2;
  };

}

#endif
