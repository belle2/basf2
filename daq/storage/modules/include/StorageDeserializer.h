//+
// File : StorageDeserializer.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#ifndef STORAGEDESERIALIZER_H
#define STORAGEDESERIALIZER_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/readout/ProcessStatusBuffer.h>
#include <daq/storage/BinData.h>

#include "daq/storage/modules/StorageRBufferManager.h"
#include "daq/storage/modules/DataStorePackage.h"

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <string>
#include <vector>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class StorageDeserializerModule : public Module {

  public:

    // Public functions
  public:
    //! Constructor / Destructor
    StorageDeserializerModule();
    virtual ~StorageDeserializerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void queueEvent();
    void storeEvent();

  private:
    DataStorePackage* m_package_q;
    unsigned int m_package_length;
    unsigned int m_package_i;
    std::string m_inputbufname;
    RingBuffer* m_inputbuf;
    StorageRBufferManager* m_buf;
    MsgHandler* m_msghandler;
    DataStoreStreamer* m_streamer;
    ProcessStatusBuffer m_status;
    bool m_running;
    int m_compressionLevel;
    int m_nrecv;
    BinData m_data;
    BinData m_data_hlt;
    BinData m_data_pxd;
    int m_nodeid;
    std::string m_nodename;
    int m_shmflag;
    int m_numThread;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
