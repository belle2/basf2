//+
// File : ReceiveEvent.h
// Description : Module to restore DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#ifndef RECEIVE_EVENT_MODULE_H
#define RECEIVE_EVENT_MODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <daq/dataflow/EvtSocket.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/pcore/DataStoreStreamer.h>

#include <TSystem.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class ReceiveEventModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    ReceiveEventModule();
    virtual ~ReceiveEventModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! Receiver Port
    std::string m_host;
    int m_port;

    //! Reciever Socket
    EvtSocketSend* m_recv;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
