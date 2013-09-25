#ifndef _B2DAQ_HostCommunicator_hh
#define _B2DAQ_HostCommunicator_hh

#include <util/IOException.hh>

#include "RunControlMessage.hh"

namespace B2DAQ {

  class HostCommunicator {
  public:
    HostCommunicator() {}
    virtual ~HostCommunicator() throw() {}

  public:
    void run();

  public:
    virtual bool init() throw(IOException) = 0;
    virtual bool reset() throw() = 0;
    virtual RunControlMessage waitMessage() throw(IOException) = 0;
    virtual void sendMessage(const RunControlMessage& message) 
      throw(IOException) = 0;
    virtual bool isReady() { return _is_ready; }

  protected:
    bool _is_ready;
    
  };

}

#endif

