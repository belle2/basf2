/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef STORAGEDESERIALIZER_H
#define STORAGEDESERIALIZER_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawCOPPER.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/storage/SharedEventBuffer.h>
#include "daq/storage/modules/DataStorePackage.h"

#include <string>
#include <vector>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class StorageDeserializerModule : public Module {

    /*
    private:
    static StorageDeserializerModule* g_module;

    public:
    static StorageDeserializerModule* get() { return g_module; }
    static DataStorePackage& getPackage() { return *(g_module->m_package); }
    static RunInfoBuffer& getInfo() { return g_module->m_info; }
    static EvtMessage* streamDataStore();
    */

    // Public functions
  public:
    //! Constructor / Destructor
    StorageDeserializerModule();
    virtual ~StorageDeserializerModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

  private:
    DataStorePackage* m_package;
    std::string m_ibuf_name;
    int m_ibuf_size;
    SharedEventBuffer m_ibuf;
    RunInfoBuffer m_info;
    int m_compressionLevel;
    int m_nodeid;
    std::string m_nodename;
    int m_shmflag;
    unsigned long long m_count;
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_evtno;
    unsigned long long m_datasize;
    MsgHandler* m_handler;
    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;
    int m_eb2;
    std::vector<std::string> m_saveObjs;

    StoreArray<RawCOPPER> rawcprarray;
    StoreArray<RawPXD> rawpxdarray;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
