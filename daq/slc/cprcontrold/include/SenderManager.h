#ifndef _Belle2_SenderManager_hh
#define _Belle2_SenderManager_hh

#include <base/COPPERNode.h>

namespace Belle2 {

  class SenderManager {

  public:
    SenderManager(COPPERNode* node) : _node(node) {}
    virtual ~SenderManager() {}

  public:
    virtual void run() throw();
    std::string run_script(const std::string& cmd);

  private:
    COPPERNode* _node;

  };

}

#endif
