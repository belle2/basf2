#ifndef _B2DAQ_EventBuilderManager_hh
#define _B2DAQ_EventBuilderManager_hh

#include <node/RONode.hh>

namespace B2DAQ {

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
