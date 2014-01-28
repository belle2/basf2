//+
// File : PXDReadRawONSEN.h
// Description : Module to receive PXD Data from external socket and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.

// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Aug - 2013
//-

#ifndef PXDREADRAWONSEN_H
#define PXDREADRAWONSEN_H


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
#include <rawdata/dataobjects/RawPXD.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class PXDReadRawONSENModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PXDReadRawONSENModule();
    virtual ~PXDReadRawONSENModule();

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

    //Sender Name
    std::string m_filename;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nread;

    //! buffer
    int* m_buffer;

    // For monitoring
    // int events_processed;

    FILE* fh;

    int readOneEvent(void);
    int read_data(char* data, size_t len);
  };

} // end namespace Belle2

#endif // PXDEADdRAWONSEN_H
