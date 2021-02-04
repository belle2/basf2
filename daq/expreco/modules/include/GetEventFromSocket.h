//+
// File : GetEventFromSocket.h
// Description : Module to receive event from socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Jan - 2021
//-

#ifndef GET_EVENT_FROM_SOCKET_MODULE_H
#define GET_EVENT_FROM_SOCKET_MODULE_H

#include <framework/core/Module.h>
#include <daq/dataflow/EvtSocket.h>

#include <string>

#include <framework/pcore/DataStoreStreamer.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class GetEventFromSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    GetEventFromSocketModule();
    virtual ~GetEventFromSocketModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    EvtMessage* receive();

    // Data members
  private:

    //! Receiver Port
    std::vector<std::string> m_hosts;
    std::vector<int> m_ports;

    //! Reciever Socket
    std::vector<EvtSocketSend*> m_socks;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
