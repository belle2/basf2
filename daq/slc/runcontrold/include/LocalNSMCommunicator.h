#ifndef _Belle2_LocalNSMCommunicator_hh
#define _Belle2_LocalNSMCommunicator_hh

#include "HostCommunicator.h"

#include <system/TCPServerSocket.h>
#include <system/TCPSocketWriter.h>
#include <system/TCPSocketReader.h>

namespace Belle2 {

  class LocalNSMCommunicator : public HostCommunicator {

  public:
    LocalNSMCommunicator(NSMCommunicator* nsm_comm)
      : _nsm_comm(nsm_comm) {}
    virtual ~LocalNSMCommunicator() throw() {}

  public:
    virtual bool init() throw(IOException);
    virtual bool reset() throw();
    virtual RunControlMessage waitMessage() throw(IOException);
    virtual void sendMessage(const RunControlMessage& message)
    throw(IOException);

  private:
    NSMCommunicator* _nsm_comm;

  };

}

#endif

