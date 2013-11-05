#ifndef _Belle2_SenderManager_hh
#define _Belle2_SenderManager_hh

#include "base/NSMNode.h"

namespace Belle2 {

  class SenderManager {

  public:
    SenderManager(NSMNode* node) : _node(node) {}
    virtual ~SenderManager() {}

  public:
    virtual void run() throw();
    std::string run_script(const std::string& cmd);

  private:
    NSMNode* _node;

  };

}

#endif
