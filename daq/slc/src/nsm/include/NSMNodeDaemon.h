#ifndef _Belle2_NSMNodeDaemon_hh
#define _Belle2_NSMNodeDaemon_hh

#include "NSMCallback.h"
#include "NSMData.h"

#include "base/NSMNode.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon(NSMNode* node = NULL, NSMCallback* callback = NULL)
      : _node(node), _callback(callback), _rdata(NULL), _wdata(NULL) {}
    virtual ~NSMNodeDaemon() throw() {}

  public:
    void setReadData(NSMData* rdata) { _rdata = rdata; }
    void setWriteData(NSMData* wdata) { _wdata = wdata; }
    void run() throw();

  private:
    NSMNode* _node;
    NSMCallback* _callback;
    NSMData* _rdata;
    NSMData* _wdata;

  };

}

#endif
