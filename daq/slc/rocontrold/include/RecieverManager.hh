#ifndef _B2DAQ_RecieverManager_hh
#define _B2DAQ_RecieverManager_hh

#include <node/RONode.hh>

namespace B2DAQ {

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
