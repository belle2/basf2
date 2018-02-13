//+
// File : PXDReadRawONSEN.h
// Description : Module to Load Raw PXD Data from ONSEN network-dump file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 15 - Apr - 2014
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <string>
#include <vector>
#include <stdlib.h>


namespace Belle2 {

  namespace PXD {

    /*! A class definition of an input module for Sequential ROOT I/O */


    class PXDReadRawONSENModule : public Module {
      enum {MAXEVTSIZE = 4 * 1024 * 1024};

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

      //! Event Meta Data
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      //! File Name
      std::string m_filename;

      //! Message handler
      MsgHandler* m_msghandler;

      //! Compression Level
      int m_compressionLevel;

      //! No. of sent events
      int m_nread;

      //! buffer
      int* m_buffer;

      //! File handle
      FILE* fh;

      int readOneEvent(void);//! Read data of one Event from File
      int read_data(char* data, size_t len);//! Read amount of data (len bytes) from file to ptr data
      bool getTrigNr(RawPXD& px);
      bool unpack_dhc_frame(void* data);
    };

  } // end namespace PXD
} // end namespace Belle2

