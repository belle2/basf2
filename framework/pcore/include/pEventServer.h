#ifndef P_EVENT_SERVER_H
#define P_EVENT_SERVER_H
//+
// File : pEventServer.h
// Description : Base class for pbasf2 input module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 1 - Jun - 2010
//-

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {
  /*! A base class to construct an event_server. Supposed to substitute Module.h. */
  class pEventServer : public Module {
  public:
    // Constructor and Destructor
    /*! Constructor */
    pEventServer();
    /*! Destructor */
    virtual ~pEventServer();

    // Member functions (for event processing)
    virtual void initialize() {};
    virtual void beginRun() {};
    /*! event function */
    /*!
      This function is supposed to pick up an event from RingBugger and restore
      DataStore contents from the streamed object.
    */
    virtual void event() {};
    virtual void endRun() {};
    virtual void terminate() {};

    // Event server functions
    /*! Event server function to be forked off before starting event processing */
    /*!
        This function is supposed to read event source repeatedly until the end
    and put event data in RingBuffer.
    */
    virtual void event_server() {};
  };

}
#endif // P_EVENT_SERVER_H



