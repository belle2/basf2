//+
// File : StorageDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/storage/modules/StorageDeserializer.h>

#include "daq/storage/storager_data.h"
#include "daq/storage/modules/StorageWorker.h"
#include "daq/storage/modules/DataStorePackage.h"

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

#include <framework/datastore/StoreArray.h>

#include <daq/slc/base/Debugger.h>
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

RunInfoBuffer* StorageDeserializerModule::g_info = NULL;
DataStorePackage* StorageDeserializerModule::g_package = NULL;

StorageDeserializerModule::StorageDeserializerModule() : Module()
{
  setDescription("Encode DataStore into RingBuffer");

  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("InputBufferName", m_inputbufname, "Name of RingBuffer", std::string(""));
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);
  addParam("NumThreads", m_numThread, "Number of threads for object decoding", 1);

  m_inputbuf = NULL;
  m_nrecv = 0;
  m_running = false;
  m_package_q = NULL;
  m_package_length = m_package_i = 0;
  m_package = new DataStorePackage();
  g_package = m_package;
  B2INFO("StorageDeserializer: Constructor done.");
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  B2INFO("StorageDeserializer: initialize() started.");
  m_shared = new SharedEventBuffer();
  m_shared->open(m_inputbufname, 1000000);
  //m_inputbuf = new RingBuffer(m_inputbufname.c_str());
  if (m_buf == NULL) {
    m_buf = new StorageRBufferManager(m_inputbuf);
  }
  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_info = new RunInfoBuffer();
      m_info->open(m_nodename, sizeof(storager_data) / 4);
      g_info = m_info;
    }
  }
  StoreArray<RawPXD>::registerPersistent();
  if (m_shmflag > 0) {
    m_info->reportRunning();
    m_running = true;
  }
  char* evtbuf = new char[10000000];
  m_data.setBuffer(evtbuf);
  int size = 0;
  while (true) {
    //while ((size = m_inputbuf->remq((int*)evtbuf)) == 0) {
    while ((size = m_shared->read((int*)evtbuf)) == 0) {
      usleep(20);
    }
    MsgHandler handler(m_compressionLevel);
    if (m_package->decode(handler, m_data)) {
      m_package->restore();
      break;
    }
  }
  //for (int n = 0; n < m_numThread; n++) {
  //  PThread(new StorageWorker(m_buf, m_compressionLevel));
  //}

  B2INFO("StorageDeserializer: initialize() done.");
}

void StorageDeserializerModule::event()
{
  while (true) {
    m_nrecv++;
    int size = 0;
    while (true) {
      //while ((size = m_inputbuf->remq((int*)m_data.getBuffer())) == 0) {
      while ((size = m_shared->read((int*)m_data.getBuffer())) == 0) {
        usleep(20);
      }
      MsgHandler handler(m_compressionLevel);
      if (m_package->decode(handler, m_data)) {
        m_package->restore();
        break;
      }
    }
    /*
      if (m_package_i == StorageWorker::MAX_QUEUES) {
      m_package_i = 0;
      }
      StorageWorker::lock();
      DataStorePackage& package(StorageWorker::getQueue()[m_package_i++]);
      while (package.getSerial() == 0) {
      StorageWorker::wait();
      }
      m_package->copy(package);
      package.setSerial(0);
      StorageWorker::notify();
      StorageWorker::unlock();
      m_package->restore();
    */
    StoreObjPtr<EventMetaData> evtmetadata;
    if (evtmetadata.isValid()) {
      m_expno = evtmetadata->getExperiment();
      m_runno = evtmetadata->getRun();
      m_evtno = evtmetadata->getEvent();
      break;
    } else {
      B2WARNING("NO event meta data " << m_data.getExpNumber() << "." << m_data.getRunNumber() << "." <<
                m_data.getEventNumber() << " nword = " << m_data.getWordSize());
      B2WARNING("Last event meta data " << m_expno << "." << m_runno << "." << m_evtno);
      DataStore::Instance().reset();
    }
  }
}

void StorageDeserializerModule::beginRun()
{
  B2INFO("StorageDeserializer: beginRun called.");
}

void StorageDeserializerModule::endRun()
{
  B2INFO("StorageDeserializer: endRun done.");
}


void StorageDeserializerModule::terminate()
{
  B2INFO("StorageDeserializer: terminate called")
}


