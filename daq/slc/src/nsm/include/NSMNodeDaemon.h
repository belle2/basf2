#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "NSMCallback.h"
#include "NSMData.h"

#include "base/NSMNode.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMNode* node = NULL, NSMCallback* callback = NULL,
                  NSMData* rdata = NULL, NSMData* wdata = NULL)
      : _node(node), _callback(callback), _rdata(rdata), _wdata(wdata) {}
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void run() throw();

  private:
    NSMNode* _node;
    NSMCallback* _callback;
    NSMData* _rdata;
    NSMData* _wdata;

  };

}

#endif
