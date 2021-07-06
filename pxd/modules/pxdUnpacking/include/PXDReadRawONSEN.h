/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
      //! Destructor
      ~PXDReadRawONSENModule();


    private:

      //! Initialize
      void initialize() override final;

      //! Event
      void event() override final;

      //! Terminate
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

