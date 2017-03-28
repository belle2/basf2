//+
// File : DepfetPFDeserializer.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#ifndef DEPFETPFDESERIALIZER_H
#define DEPFETPFDESERIALIZER_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/storage/depfet/dataobjects/RawDepfetPF.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/storage/SharedEventBuffer.h>
#include <daq/storage/BinData.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>

#include <string>
#include <vector>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class DepfetPFDeserializerModule : public Module {

    // Public functions
  public:
    //! Constructor / Destructor
    DepfetPFDeserializerModule();
    virtual ~DepfetPFDeserializerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    std::string m_ibuf_name;
    int m_ibuf_size;
    std::vector<SharedEventBuffer> m_ibuf;
    RunInfoBuffer m_info;
    int m_compressionLevel;
    int m_nodeid;
    std::string m_nodename;
    int m_shmflag;
    unsigned long long m_count;
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_evtno;
    int* m_evtbuf;
    unsigned long long m_datasize;
    std::vector<std::string> m_saveObjs;
    StoreObjPtr<EventMetaData> m_evtmeta;
    StoreArray<RawDepfetPF> m_rawdata;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
