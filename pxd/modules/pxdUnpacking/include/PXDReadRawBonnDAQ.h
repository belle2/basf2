//+
// File : PXDReadRawBonnDAQ.h
// Description : Module to Load BonnDAQ file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 02.05.2018
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

    class PXDReadRawBonnDAQModule : public Module {
      enum {MAXEVTSIZE = 4 * 1024 * 1024 + 256 * 4 + 16};
      // Public functions
    public:

      //! Constructor / Destructor
      PXDReadRawBonnDAQModule();

    private:

      ~PXDReadRawBonnDAQModule() override final;

      void initialize() override final;
      void event() override final;
      void terminate() override final;


      // Data members

      // Parallel processing parameters

      //! Event Meta Data
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      //! DHH Data
      StoreArray<RawPXD> m_rawPXD;

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

      unsigned int m_expNr;// set by Param
      unsigned int m_runNr;// set by Param

      void endian_swapper(void* a, unsigned int len);
      int readOneEvent(void);//! Read data of one Event from File
      int read_data(char* data, size_t len);//! Read amount of data (len bytes) from file to ptr data
    };

  } // end namespace PXD
} // end namespace Belle2

