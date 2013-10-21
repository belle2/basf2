//+
// File : DeSerializerPXD.h
// Description : Module to receive PXD Data from external socket and store it as RawPXD in Data Store
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Aug - 2013
//-

#ifndef DESERIALIZERPXD_H
#define DESERIALIZERPXD_H


#include <string>
#include <vector>
#include <stdlib.h>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/dataobjects/EventMetaData.h>
#include <pxd/dataobjects/RawPXD.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerPXDModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerPXDModule();
    virtual ~DeSerializerPXDModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    // Parallel processing parameters

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! Receiver Port
    int m_port;

    //Sender Name
    std::string m_host;

    //! Reciever Socket
    EvtSocketSend* m_recv;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nsent;

    //! buffer
    int* m_buffer;

    // For monitoring
    timeval m_t0;
    double m_totbytes;
    int m_ncycle;
    int events_processed;

  };

} // end namespace Belle2

#endif // DESERIALIZERPXD_H