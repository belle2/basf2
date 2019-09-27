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
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <string>
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
      ~PXDReadRawONSENModule();


    private:

      //! Module functions to be called from main process
      void initialize() override final;

      //! Module functions to be called from event process
      void event() override final;
      void terminate() override final;


      // Data members

      //! Event Meta Data
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      //! Set Event Meta Info
      bool m_setEvtMeta;
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

      int readOneEvent(void); //!< Read data of one Event from File
      int read_data(char* data, size_t len); //!< Read amount of data (len bytes) from file to ptr data
      bool getTrigNr(RawPXD& px); //!< get the trigger number
      bool unpack_dhc_frame(void* data); //!< unpack the dhc frame
    };

  } // end namespace PXD
} // end namespace Belle2

