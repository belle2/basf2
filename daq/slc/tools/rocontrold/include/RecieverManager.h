#ifndef _Belle2_RecieverManager_hh
#define _Belle2_RecieverManager_hh

#include "base/NSMNode.h"

namespace Belle2 {

  class RecieverManager {

  public:
    RecieverManager(NSMNode* node = NULL)
      : _node(node) {}
    virtual ~RecieverManager() {}

  public:
    virtual void run() throw();

  private:
    NSMNode* _node;

  };

}

#endif
