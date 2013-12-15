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

#include <daq/slc/readout/ProcessStatusBuffer.h>
#include <daq/storage/BinData.h>

#include <string>
#include <vector>

#define MAXEVTSIZE  400000000

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class StorageDeserializerModule : public Module {

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

  private:
    void storeEvent();

  private:
    int m_evtbuf[MAXEVTSIZE];
    std::string m_inputbufname;
    std::string m_nsmnodename;
    int m_nsmnodeid;
    RingBuffer* m_inputbuf;
    MsgHandler* m_msghandler;
    DataStoreStreamer* m_streamer;
    ProcessStatusBuffer m_status;
    bool m_running;
    int m_compressionLevel;
    int m_nrecv;
    BinData m_data;
    BinData m_data_hlt;
    BinData m_data_pxd;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
