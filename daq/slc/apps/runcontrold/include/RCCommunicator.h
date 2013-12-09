#ifndef _Belle2_RCCommunicator_h
#define _Belle2_RCCommunicator_h

#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/NSMNode.h>
#include <daq/slc/base/DataObject.h>

#include "daq/slc/apps/runcontrold/RunControlMessage.h"

namespace Belle2 {

  class RCCommunicator {

  public:
    RCCommunicator() throw();
    virtual ~RCCommunicator() throw();

  public:
    virtual bool sendMessage(const RunControlMessage& msg) throw() = 0;
    virtual bool sendState(NSMNode* node) throw() = 0;
    virtual bool sendDataObject(const std::string& name,
                                DataObject* data) throw() = 0;
    virtual bool isOnline(NSMNode* node) throw() = 0;
    bool isReady() throw();
    void setReady(bool ready) throw();

  private:
    bool _is_ready;
    Mutex _mutex;

  };

}

#endif
