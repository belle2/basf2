
//+
// File : StorageDeserializer.cc
// Description : Module to receive data from eb2rx and store online disk
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#include <daq/storage/modules/StorageDeserializer.h>
#include <TSystem.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageDeserializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StorageDeserializerModule::StorageDeserializerModule() : Module()
{
  setDescription("Encode DataStore into RingBuffer");

  addParam("InputBufferName", m_inputbufname, "Name of RingBuffer", std::string(""));
  addParam("CompressionLevel", m_compressionLevel, "Compression level", 0);
  addParam("NodeID", m_nodeid, "Node(subsystem) ID", 0);
  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));
  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  m_inputbuf = NULL;
  m_nrecv = 0;
  m_compressionLevel = 0;
  m_running = false;

  B2INFO("StorageDeserializer: Constructor done.");
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  B2INFO("StorageDeserializer: initialize() started.");
  B2INFO(m_inputbufname.c_str());

  m_inputbuf = new RingBuffer(m_inputbufname.c_str());
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  StoreArray<RawPXD>::registerPersistent();
  m_nrecv = -1;

  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      m_status.open(m_nodename, m_nodeid);
      m_status.reportReady();
    }
  }
  m_data.setBuffer(m_evtbuf);
  storeEvent();
  if (m_shmflag > 0) {
    m_status.reportRunning();
    m_running = true;
  }
  B2INFO("StorageDeserializer: initialize() done.");
}

void StorageDeserializerModule::event()
{
  m_nrecv++;
  storeEvent();
}

void StorageDeserializerModule::beginRun()
{
  B2INFO("StorageDeserializer: beginRun called.");
}

void StorageDeserializerModule::endRun()
{
  //fill Run data
  B2INFO("StorageDeserializer: endRun done.");
}


void StorageDeserializerModule::terminate()
{
  B2INFO("StorageDeserializer: terminate called")
}

void StorageDeserializerModule::storeEvent()
{
  int size;
  bool tried = false;
  static int count = 0;
  while ((size = m_inputbuf->remq(m_data.getBuffer())) == 0) {
    if (m_shmflag > 0 && !tried) {
      //m_status.reportWarning("Ring buffer empty. waiting to be ready");
      //B2WARNING("Ring buffer empty. waiting to be ready");
      tried = true;
    }
    usleep(200);
  }
  m_data_hlt.setBuffer(m_data.getBody());
  EvtMessage* evtmsg = new EvtMessage((char*)m_data_hlt.getBody());
  if (m_data.getBodyByteSize() > m_data_hlt.getByteSize()) {
    m_data_pxd.setBuffer(m_data.getBody() + m_data_hlt.getWordSize());
    StoreArray<RawPXD> rawpxdary;
    RawPXD rawpxd((int*)m_data_pxd.getBody(), m_data_pxd.getByteSize());
    rawpxdary.appendNew(rawpxd);
  }
  m_streamer->restoreDataStore(evtmsg);
  delete evtmsg;
  if (count % 1000 == 0) {
    printf("record %d evt no = %d size = %d\n", count,
           m_data.getEventNumber(), m_data.getWordSize());
  }
  /*
  if (count % 1000 == 0) {
    printf("evt no0 = %d \n", m_data.getEventNumber());
    printf("nw1     = %d ", m_data_hlt.getWordSize());
    printf("hnw1    = %d ", m_data_hlt.getHeaderWordSize());
    printf("exp no1 = %d ", m_data_hlt.getExpNumber());
    printf("run no1 = %d ", m_data_hlt.getRunNumber());
    printf("evt no1 = %d ", m_data_hlt.getEventNumber());
    printf("magic1  = %x \n", m_data_hlt.getTrailerMagic());
    printf("nw2     = %d ", m_data_pxd.getWordSize());
    printf("hnw2    = %d ", m_data_pxd.getHeaderWordSize());
    printf("exp no2 = %d ", m_data_pxd.getExpNumber());
    printf("run no2 = %d ", m_data_pxd.getRunNumber());
    printf("evt no2 = %d ", m_data_pxd.getEventNumber());
    printf("magic2  = %x \n", m_data_pxd.getTrailerMagic());
  }
  */
  count++;
}

