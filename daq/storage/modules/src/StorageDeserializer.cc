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

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           std::string("InputRbuf"));
  addParam("CompressionLevel", m_compressionLevel,
           "Compression level", 0);

  m_rbuf = NULL;
  m_nrecv = 0;
  m_compressionLevel = 0;

  B2INFO("StorageDeserializer: Constructor done.");
}


StorageDeserializerModule::~StorageDeserializerModule()
{
}

void StorageDeserializerModule::initialize()
{
  B2INFO("StorageDeserializer: initialize() started.");

  gSystem->Load("libdataobjects");

  m_rbuf = new RingBuffer(m_rbufname.c_str());
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  m_nrecv = -1;

  storeEvent();

  B2INFO("StorageDeserializer: initialize() done.");
}

void StorageDeserializerModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;
  storeEvent();
  return;
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
  while ((size = m_rbuf->remq(m_evtbuf)) == 0) {
    usleep(20);
  }

  EvtMessage* evtmsg = new EvtMessage((char*)m_evtbuf);
  m_streamer->restoreDataStore(evtmsg);
  delete evtmsg;

}

