#ifndef P_EVENT_SERVER_H
#define P_EVENT_SERVER_H
//+
// File : pEventServer.h
// Description : Base class for pbasf2 input module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 1 - Jun - 2010
//-

#include <string>

#include "framework/core/Module.h"

namespace Belle2 {
  class pEventServer : public Module {
  public:
    // Constructor and Destructor
    pEventServer();
    virtual ~pEventServer();

    // Member functions (for event processing)
    virtual void initialize() {};
    virtual void beginRun() {};
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    // Event server functions
    virtual void event_server() {};
  };

}
#endif // P_EVENT_SERVER_H



