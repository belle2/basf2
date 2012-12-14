//+
// File : DeSerializer.h
// Description : Module to restore DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Jan - 2012
//-

#ifndef DESERIALIZER_H
#define DESERIALIZER_H

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
#include <daq/dataobjects/RawCDC.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerModule();
    virtual ~DeSerializerModule();

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
    std::vector<int> m_port;

    //! Reciever Socket
    std::vector<EvtSocketRecv*> m_recv;

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


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
