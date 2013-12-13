
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
  //gSystem->Load("libpxd_dataobjects");
  //gSystem->Load("libdataobjects");

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

  m_rbuf = new RingBuffer(m_rbufname.c_str());
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  StoreArray<RawPXD>::registerPersistent();
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
  unsigned int* buf = (unsigned int*)(m_evtbuf);
  unsigned int* pbuf_hlt = buf + buf[1];
  EvtMessage* evtmsg = new EvtMessage((char*)(pbuf_hlt + pbuf_hlt[1]));
  m_streamer->restoreDataStore(evtmsg);
  unsigned int* pbuf_pxd = pbuf_hlt + pbuf_hlt[0];
  StoreArray<RawPXD> rawpxdary;
  RawPXD rawpxd((int*)(pbuf_pxd + pbuf_pxd[1]), pbuf_pxd[0] - pbuf_pxd[1]);
  rawpxdary.appendNew(rawpxd);
  delete evtmsg;

  if (buf[4] % 10 == 0) {
    printf("nw      = %d ", buf[0]);
    printf("hnw     = %d ", buf[1]);
    printf("exp_run = %d ", buf[3]);
    printf("evt no  = %d ", buf[4]);
    printf("magic   = %x ", buf[buf[0] - 1]);
    printf("nw2     = %d ", pbuf_hlt[0]);
    printf("hnw2    = %d ", pbuf_hlt[1]);
    printf("magic2  = %x ", pbuf_hlt[pbuf_hlt[0] - 1]);
    printf("nw2     = %d ", pbuf_pxd[0]);
    printf("hnw2    = %d ", pbuf_pxd[1]);
    printf("magic2  = %x \n", pbuf_pxd[pbuf_pxd[0] - 1]);
  }
}

