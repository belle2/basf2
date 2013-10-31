#ifndef _Belle2_HostCommunicator_hh
#define _Belle2_HostCommunicator_hh

#include <base/IOException.h>

#include "RunControlMessage.h"

namespace Belle2 {

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

