//+
// File : StorageSerializer.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//         9 - Dec - 2013, Modification for DAQ use
//-


#include <daq/storage/modules/StorageSerializer.h>
#include <daq/storage/modules/StorageDeserializer.h>

#include <iostream>
#include <TList.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StorageSerializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StorageSerializerModule::StorageSerializerModule() : Module()
{
  //Set module properties
  setDescription("SeqROOT output module");

  //Parameter definition
  addParam("compressionLevel", m_compressionLevel, "Compression Level", 0);
  addParam("OutputBufferName", m_obuf_name, "Output buffer name", string(""));
  addParam("OutputBufferSize", m_obuf_size, "Output buffer size", 10);
  std::cout << "[DEBUG] StorageSerializer: Constructor done." << std::endl;
}


StorageSerializerModule::~StorageSerializerModule() { }

void StorageSerializerModule::initialize()
{
  m_msghandler = new MsgHandler(m_compressionLevel);
  //m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_expno = m_runno = -1;
  m_count = m_count_0 = 0;
  if (m_obuf_name.size() > 0 && m_obuf_size > 0) {
    m_obuf.open(m_obuf_name.c_str(), m_obuf_size * 1000000);
  } else {
    B2FATAL("Failed to load arguments for shared buffer (" <<
            m_obuf_name.c_str() << ":" << m_obuf_size << ")");
  }
  std::cout << "[DEBUG] StorageSerializer: initialized." << std::endl;
}


void StorageSerializerModule::beginRun()
{
  B2INFO("StorageSerializer: beginRun called.");
}

int StorageSerializerModule::writeStreamerInfos()
{
  TList* minilist = 0 ;
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const TClass* entryClass = iter->second.objClass;
      TVirtualStreamerInfo* vinfo = entryClass->GetStreamerInfo();
      //std::cout << "Recording StreamerInfo : durability " << durability
      //  << " : Class Name " << entryClass->GetName() << std::endl;
      if (!minilist) minilist  =  new TList();
      minilist->Add((TObject*)vinfo);
    }
  }
  if (minilist) {
    m_msghandler->add(minilist, "StreamerInfo");
    EvtMessage* msg = m_msghandler->encode_msg(MSG_STREAMERINFO);
    (msg->header())->nObjects = 1;       // No. of objects
    (msg->header())->nArrays = 0;    // No. of arrays
    int nword = (msg->size() - 1) / 4 + 1;
    m_obuf.write((int*)msg->buffer(), nword, false, 0, true);
    int size = msg->size();
    B2INFO("Wrote StreamerInfo to a file : " << size << "bytes");
    delete msg;
    delete minilist;
    return size;
  } else {
    B2WARNING("No StreamerInfo in memory");
  }
  return 0;
}

void StorageSerializerModule::event()
{
  StoreObjPtr<EventMetaData> evtmetadata;
  unsigned int expno = evtmetadata->getExperiment();
  unsigned int runno = evtmetadata->getRun();
  unsigned int subno = 0;//evtmetadata->getRun() & 0xFF;
  if (StorageDeserializerModule::get()) {
    RunInfoBuffer& info(StorageDeserializerModule::getInfo());
    if (info.isAvailable()) {
      info.setExpNumber(expno);
      info.setRunNumber(runno);
      info.setSubNumber(subno);
    }
  }
  SharedEventBuffer::Header* header = m_obuf.getHeader();
  m_obuf.lock();
  if (header->runno < runno || header->expno < expno) {
    m_count = 0;
    B2INFO("New run detected: expno = " << expno << " runno = " << runno << " subno = " << subno);
    header->expno = expno;
    header->runno = runno;
    header->subno = subno;
    m_nbyte = writeStreamerInfos();
    m_obuf.unlock();
    return ;
  } else {
    m_obuf.unlock();
  }

  EvtMessage* msg = StorageDeserializerModule::streamDataStore();
  int nword = (msg->size() - 1) / 4 + 1;
  m_obuf.lock();
  m_obuf.write((int*)msg->buffer(), nword, false, 0, true);
  m_obuf.unlock();
  m_nbyte += msg->size();
  delete msg;
  if (m_count < 10000 && (m_count < 10 || (m_count > 10 && m_count < 100 && m_count % 10 == 0) ||
                          (m_count > 100 && m_count < 1000 && m_count % 100 == 0) ||
                          (m_count > 1000 && m_count < 10000 && m_count % 1000 == 0))) {
    std::cout << "[DEBUG] Storage count = " << m_count << " nword = " << nword << std::endl;
  }
  if (m_count % 10 == 0 && StorageDeserializerModule::get()) {
    RunInfoBuffer& info(StorageDeserializerModule::getInfo());
    if (info.isAvailable()) {
      info.setOutputCount(m_count);
      info.addOutputNBytes(m_nbyte);
    }
    m_nbyte = 0;
  }
  m_count++;
}

void StorageSerializerModule::endRun()
{
  std::cout << "[DEBUG] StorageSerializer : endRun called" << std::endl;
}


void StorageSerializerModule::terminate()
{
  std::cout << "[DEBUG] terminate called" << std::endl;
}

