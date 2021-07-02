/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <pxd/unpacking/PXDLocalDAQFile.h>

#include <string>
#include <stdlib.h>


namespace Belle2 {

  namespace PXD {
    //! Module to Load Raw PXD Data from DHH network-dump file and store it as RawPXD in Data Store
    //! This is meant for lab use (standalone testing, debugging) without an event builder.
    class PXDReadRawBonnDAQModule final : public Module {
      enum {MAXEVTSIZE = 4 * 1024 * 1024 + 256 * 4 + 16};

    public:
      /// Constructor
      PXDReadRawBonnDAQModule();

    private:
      /// Destructor
      ~PXDReadRawBonnDAQModule() override final;

      /** Initialize */
      void initialize() override final;

      /** Event */
      void event() override final;

      /** Terminate */
      void terminate() override final;

      // Data members

      //! Event Meta Data
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      //! DHH Data
      StoreArray<RawPXD> m_rawPXD;

      //! File Name
      std::string m_filename;

      //! buffer
      int* m_buffer;

      //! File handle
      PXDLocalDAQFile* fh;

      unsigned int m_expNr; //!< set by Param
      unsigned int m_runNr; //!< set by Param
      unsigned int m_subRunNr; //!< set by Param

      int readOneEvent(void); //! Read data of one Event from File

    };

  } // end namespace PXD
} // end namespace Belle2

