#ifndef _B2DAQ_RecieverManager_hh
#define _B2DAQ_RecieverManager_hh

#include <node/DataReceiverNode.hh>

namespace B2DAQ {

  class RecieverManager {
    
  public:
    RecieverManager(DataReceiverNode* node = NULL)
      : _node(node) {}
    virtual ~RecieverManager() {}

  public:
    virtual void run() throw();
    
  private:
    DataReceiverNode* _node;

  };

}

#endif
