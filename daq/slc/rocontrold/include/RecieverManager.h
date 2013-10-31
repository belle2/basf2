#ifndef _Belle2_RecieverManager_hh
#define _Belle2_RecieverManager_hh

#include <base/RONode.h>

namespace Belle2 {

  class RecieverManager {

  public:
    RecieverManager(RONode* node = NULL)
      : _node(node) {}
    virtual ~RecieverManager() {}

  public:
    virtual void run() throw();

  private:
    RONode* _node;

  };

}

#endif
