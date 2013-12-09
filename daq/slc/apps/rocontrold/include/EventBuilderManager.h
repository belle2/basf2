#ifndef _Belle2_EventBuilderManager_hh
#define _Belle2_EventBuilderManager_hh

#include "daq/slc/base/NSMNode.h"

namespace Belle2 {

  class EventBuilderManager {

  public:
    EventBuilderManager(NSMNode* node = NULL)
      : _node(node) {}
    virtual ~EventBuilderManager() {}

  public:
    virtual void run() throw();

  private:
    NSMNode* _node;

  };

}

#endif
