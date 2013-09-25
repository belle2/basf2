#ifndef _B2DAQ_LocalNSMCommunicator_hh
#define _B2DAQ_LocalNSMCommunicator_hh

#include "HostCommunicator.hh"

#include <system/TCPServerSocket.hh>
#include <system/TCPSocketWriter.hh>
#include <system/TCPSocketReader.hh>

namespace B2DAQ {

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

