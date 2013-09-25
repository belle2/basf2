#ifndef _B2DAQ_EventBuilderManager_hh
#define _B2DAQ_EventBuilderManager_hh

#include <node/DataReceiverNode.hh>

namespace B2DAQ {

  class EventBuilderManager {
    
  public:
    EventBuilderManager(DataReceiverNode* node = NULL)
      : _node(node) {}
    virtual ~EventBuilderManager() {}

  public:
    virtual void run() throw();
    
  private:
    DataReceiverNode* _node;

  };

}

#endif
