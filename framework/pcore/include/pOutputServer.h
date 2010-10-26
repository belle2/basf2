#ifndef P_OUTPUT_SERVER_H
#define P_OUTPUT_SERVER_H
//+
// File : pOutputServer.h
// Description : Base class for pbasf2 output module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 30 - Jun - 2010
//-

#include "framework/core/Module.h"

namespace Belle2 {
  class pOutputServer : public Module {
  public:
    // Constructor and Destructor
    pOutputServer();
    virtual ~pOutputServer();

    // Member functions (for event processing)
    virtual void initialize() {};
    virtual void beginRun() {};
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    // Event server functions
    virtual void output_server() {};
  };

}

#endif // P_OUTPUT_SERVER_H



