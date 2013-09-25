#ifndef _B2DAQ_SenderManager_hh
#define _B2DAQ_SenderManager_hh

#include <node/COPPERNode.hh>

namespace B2DAQ {

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
