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
  /*! A base class to construct an output_server. Supposed to substitute Module.h.*/
  class pOutputServer : public Module {
  public:
    // Constructor and Destructor
    /*! Constructor */
    pOutputServer();
    /*! Destructor */
    virtual ~pOutputServer();

    // Member functions (for event processing)
    virtual void initialize() {};
    virtual void beginRun() {};
    /*! event function */
    /*! Supposed to convert DataStore objects into streamed buffer and place it in RingBuffer */
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    // Output server functions
    /*! Output server function to be forked off before starting event processing */
    /*!
        This function is supposed to pick up a buffer from RingBuffer and store it into
    output source until termination record is received
    */
    virtual void output_server() {};
  };

}

#endif // P_OUTPUT_SERVER_H



