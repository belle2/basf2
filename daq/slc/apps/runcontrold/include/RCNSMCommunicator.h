#ifndef _Belle2_RCNSMCommunicator_h
#define _Belle2_RCNSMCommunicator_h

#include "daq/slc/apps/runcontrold/RCCommunicator.h"

#include <daq/slc/nsm/NSMCommunicator.h>

namespace Belle2 {

  class RCNSMCommunicator : public RCCommunicator {

  public:
    RCNSMCommunicator(NSMNode* rc_node, NSMCommunicator* comm);
    virtual ~RCNSMCommunicator() throw();

  public:
    virtual bool sendMessage(const RunControlMessage& msg) throw();
    virtual bool sendState(NSMNode* node) throw();
    virtual bool sendDataObject(const std::string& name,
                                DataObject* data) throw();
    virtual bool isOnline(NSMNode* node) throw();
    virtual void sendLog(const SystemLog& log) throw();

  private:
    NSMNode* _rc_node;
    NSMCommunicator* _comm;
    Mutex _nsm_mutex;

  };

}

#endif
