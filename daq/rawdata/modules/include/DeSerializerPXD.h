/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZERPXD_H
#define DESERIALIZERPXD_H

#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawPXD.h>

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

    // DataStore interface
    StoreArray<RawPXD> rawpxdary;

    // Parallel processing parameters

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! # of connections
    int m_num_connections;

    //! Receiver Port
    std::vector<int> m_ports;

    //! Sender Names
    std::vector<std::string> m_hosts;

    //! Receiver Sockets
    std::vector<EvtSocketSend*> m_recvs;

    //! Compression Level
    int m_compressionLevel;

    //! No. of rcvd events
    int m_nEvents;

    //! buffer
    int* m_buffer;

    // For monitoring
    // int events_processed;

  };

} // end namespace Belle2

#endif // DESERIALIZERPXD_H
