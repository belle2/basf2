//+
// File : txsocket.h
// Description : Module to send DataStore via EvtSocket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef TXMODULE_H
#define TXMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <daq/dataflow/EvtSocket.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/pcore/DataStoreStreamer.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TxSocketModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    TxSocketModule();
    virtual ~TxSocketModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //!Compression parameter
    int m_compressionLevel;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_dest;

    //! Destination port
    int m_port;

    //! EvtSocket
    EvtSocketSend* m_sock;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
