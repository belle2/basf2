#ifndef _Belle2_LocalNSMCommunicator_hh
#define _Belle2_LocalNSMCommunicator_hh

#include "base/Command.h"

#include "system/TCPServerSocket.h"
#include "system/TCPSocketWriter.h"
#include "system/TCPSocketReader.h"

#include "nsm/NSMCommunicator.h"
#include "nsm/NSMHandlerException.h"

#include "RunControlMessage.h"

#include <vector>
#include <map>

namespace Belle2 {

  class LocalNSMCommunicator {

  public:
    LocalNSMCommunicator(NSMCommunicator* gcom, NSMCommunicator* lcom)
    throw (NSMHandlerException);
    ~LocalNSMCommunicator() throw() {}

  public:
    int sendMessage(NSMNode* node, RunControlMessage& message)
    throw(IOException);
    void run();

  private:
    std::vector<NSMCommunicator*> _comm_v;
    std::map<NSMNode*, NSMCommunicator*> _comm_m;


  };

}

#endif

