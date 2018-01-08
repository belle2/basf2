//+
// File : StorageDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/storage/modules/StorageDeserializer.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawPXD.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/Time.h>

#include <TSystem.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageDeserializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//StorageDeserializerModule* StorageDeserializerModule::g_module = NULL;
//
//EvtMessage* StorageDeserializerModule::streamDataStore()
//{
//  return g_module->m_streamer->streamDataStore(DataStore::c_Event);
//}

StorageDeserializerModule::StorageDeserializerModule() : Module()
{
  setDescription("Storage deserializer module");

  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("EB2", m_eb2, "Over capsuled by eb2", 1);
  addParam("InputBufferName", m_ibuf_name, "Input buffer name", std::string(""));
  addParam("InputBufferSize", m_ibuf_size, "Input buffer size", 100);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  m_count = 0;
  //g_module = this;
  B2DEBUG(100, "StorageDeserializer: Constructor done.");
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  std::cout << "StorageDeserializer: initialize() started." << std::endl;
  if (m_ibuf_name.size() > 0 && m_ibuf_size > 0) {
    m_ibuf.open(m_ibuf_name, m_ibuf_size * 1000000);
  } else {
    B2FATAL("Failed to load arguments for shared buffer (" <<
            m_ibuf_name.c_str() << ":" << m_ibuf_size << ")");
  }
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_info.open(m_nodename, m_nodeid);
    }
  }
  m_handler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer();
  m_package = new DataStorePackage(m_streamer, m_eb2);

  StoreArray<RawPXD>::registerPersistent();
  if (m_info.isAvailable()) {
    m_info.reportReady();
  }
  m_count = 0;
  while (true) {
    m_package->setSerial(m_ibuf.read((int*)m_package->getData().getBuffer(), true, false));
    if (m_package->restore()) {
      if (m_info.isAvailable()) {
        m_info.setInputNBytes(m_package->getData().getByteSize());
        m_info.setInputCount(1);
      }
      break;
    }
  }
  if (m_info.isAvailable()) {
    m_info.reportReady();
  }
  std::cout << "StorageDeserializer: initialize() done." << std::endl;
}

void StorageDeserializerModule::event()
{
  m_count++;
  if (m_count == 1) return;
  while (true) {
    m_package->setSerial(m_ibuf.read((int*)m_package->getData().getBuffer(), true, false));
    if (m_package->restore()) {
      if (m_info.isAvailable()) {
        m_info.addInputNBytes(m_package->getData().getByteSize());
        m_info.setInputCount(m_count);
      }
      break;
    }
  }
  StoreObjPtr<EventMetaData> evtmetadata;
  if (evtmetadata.isValid()) {
    if (m_expno != evtmetadata->getExperiment() ||
        m_runno != evtmetadata->getRun()) {
      if (m_info.isAvailable()) {
        m_info.setInputNBytes(m_package->getData().getByteSize());
        m_info.setInputCount(1);
      }
    }
    m_expno = evtmetadata->getExperiment();
    m_runno = evtmetadata->getRun();
    m_evtno = evtmetadata->getEvent();
    if (m_info.isAvailable()) {
      m_info.setExpNumber(m_expno);
      m_info.setRunNumber(m_runno);
    }
  } else {
    B2WARNING("NO event meta data " << m_package->getData().getExpNumber() << "." <<
              m_package->getData().getRunNumber() << "." <<
              m_package->getData().getEventNumber() << " nword = " <<
              m_package->getData().getWordSize());
    B2WARNING("Last event meta data " << m_expno << "." << m_runno << "." << m_evtno);
  }
}

void StorageDeserializerModule::beginRun()
{
  std::cout << "StorageDeserializer: beginRun called." << std::endl;
}

void StorageDeserializerModule::endRun()
{
  std::cout << "StorageDeserializer: endRun done." << std::endl;
}


void StorageDeserializerModule::terminate()
{
  std::cout << "StorageDeserializer: terminate called" << std::endl;
}


