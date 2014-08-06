//+
// File : storageoutput.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//         9 - Dec - 2013, Modification for DAQ use
//-


#include <daq/storage/modules/StorageOutput.h>
#include <daq/storage/modules/StorageDeserializer.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StorageOutputModule::StorageOutputModule() : Module()
{
  //Set module properties
  setDescription("SeqROOT output module");

  //Parameter definition
  addParam("compressionLevel", m_compressionLevel, "Compression Level", 1);
  addParam("OutputBufferName", m_obuf_name, "Output buffer name", string(""));
  addParam("OutputBufferSize", m_obuf_size, "Output buffer size", 100000000);
  B2DEBUG(1, "StorageOutput: Constructor done.");
}


StorageOutputModule::~StorageOutputModule() { }

void StorageOutputModule::initialize()
{
  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_expno = m_runno = -1;
  m_count = m_count_0 = 0;
  if (m_obuf_name.size() > 0 && m_obuf_size > 0) {
    m_obuf.open(m_obuf_name.c_str(), m_obuf_size * 1000000);
  } else {
    B2ERROR("Failed to load arguments for shared buffer (" <<
            m_obuf_name.c_str() << ":" << m_obuf_size << ")");
  }
  B2INFO("StorageOutput: initialized.");
}


void StorageOutputModule::beginRun()
{
  B2INFO("StorageOutput: beginRun called.");
}

void StorageOutputModule::event()
{
  StoreObjPtr<EventMetaData> evtmetadata;
  unsigned int expno = evtmetadata->getExperiment();
  unsigned int runno = evtmetadata->getRun();
  unsigned int subno = 0;//evtmetadata->getRun() & 0xFF;
  SharedEventBuffer::Header* header = m_obuf.getHeader();
  if (header->runno < runno || header->expno < expno) {
    m_obuf.lock();
    m_count = 0;
    header->expno = expno;
    header->runno = runno;
    header->subno = subno;
    m_obuf.unlock();
    RunInfoBuffer& info(StorageDeserializerModule::getInfo());
    if (info.isAvailable()) {
      info.setExpNumber(expno);
      info.setRunNumber(runno);
      info.setSubNumber(subno);
      info.setOutputCount(0);
      info.setOutputNBytes(0);
    }
  }
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);
  int nword = (msg->size() - 1) / 4 + 1;
  m_obuf.write((int*)msg->buffer(), nword,
               false, StorageDeserializerModule::getPackage().getSerial());
  m_nbyte += msg->size();
  delete msg;
  if (m_count < 10000 && (m_count < 10 || (m_count > 10 && m_count < 100 && m_count % 10 == 0) ||
                          (m_count > 100 && m_count < 1000 && m_count % 100 == 0) ||
                          (m_count > 1000 && m_count < 10000 && m_count % 1000 == 0))) {
    B2INFO("Storage count = " << m_count);
  }
  if (m_count % 10 == 0) {
    RunInfoBuffer& info(StorageDeserializerModule::getInfo());
    if (info.isAvailable()) {
      info.setOutputCount(m_count);
      info.addOutputNBytes(m_nbyte);
    }
    m_count_0 = m_count;
    m_nbyte = 0;
  }
  m_count++;
}

void StorageOutputModule::endRun()
{
  B2INFO("StorageOutput : endRun called");
}


void StorageOutputModule::terminate()
{
  B2INFO("terminate called")
}

