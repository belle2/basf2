//+
// File : txsocket.h
// Description : Module to send DataStore via EvtSocket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/dataobjects/RawCDC.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class SerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    SerializerModule();
    virtual ~SerializerModule();

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

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int m_nsent;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_dest;

    //! Destination port
    int m_port;

    //! EvtSocket
    EvtSocketSend* m_sock;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
