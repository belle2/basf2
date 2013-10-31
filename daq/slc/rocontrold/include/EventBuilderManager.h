#ifndef _Belle2_EventBuilderManager_hh
#define _Belle2_EventBuilderManager_hh

#include <base/RONode.h>

namespace Belle2 {

  class EventBuilderManager {

  public:
    EventBuilderManager(RONode* node = NULL)
      : _node(node) {}
    virtual ~EventBuilderManager() {}

  public:
    virtual void run() throw();

  private:
    RONode* _node;

  };

}

#endif
