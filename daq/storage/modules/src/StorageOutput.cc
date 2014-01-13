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
#include <daq/storage/modules/MonitorStorage.h>

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
  //  setPropertyFlags(c_Output | c_ParallelProcessingCertified);
  m_file = 0;
  m_msghandler = 0;
  m_streamer = 0;

  //Parameter definition
  addParam("compressionLevel", m_compressionLevel, "Compression Level", 0);
  addParam("OutputBufferName", m_obufname, "Name of Ring Buffer for express reco", string(""));
  addParam("DumpInterval", m_interval, "Event interval to send express reco", 10);
  addParam("StorageDir", m_stordir, "Directory to write output files", string(""));
  B2DEBUG(1, "StorageOutput: Constructor done.");
}


StorageOutputModule::~StorageOutputModule() { }

void StorageOutputModule::initialize()
{
  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  m_expno = -1;
  m_runno = -1;

  if (m_obufname.size() > 0) {
    m_obuf = new RingBuffer(m_obufname.c_str(), 10000000);
    m_obuf->clear();
  } else
    m_obuf = NULL;

  B2INFO("StorageOutput: initialized.");
}


void StorageOutputModule::beginRun()
{
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevts = 0;

  B2INFO("StorageOutput: beginRun called.");
}

void StorageOutputModule::event()
{
  StoreObjPtr<EventMetaData> evtmetadata;
  int expno = evtmetadata->getExperiment();
  int runno = evtmetadata->getRun() >> 8;
  int subno = evtmetadata->getRun() & 0xFF;
  if (m_runno != runno || m_expno != expno) {
    storager_data& data(MonitorStorageModule::getData());
    if (m_file != NULL) {
      delete m_file;
      m_file = NULL;
    }
    m_expno = expno;
    m_runno = runno;
    data.expno = expno;
    data.runno = runno;
    data.subno = subno;
    Time t;
    data.starttime = t.getSecond();
    data.curtime = t.get();
    m_file = openDataFile();
    m_nevts = 0;
  }

  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());
  if (m_obuf != NULL) {
    if (m_nevts % m_interval == 0) {
      m_obuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    }
  }

  // Statistics
  double dsize = (double)stat / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevts++;
  delete msg;

  m_datasize += stat;
  if (m_nevts % 10000 == 0) {
    storager_data& data(MonitorStorageModule::getData());
    Time t;
    double curtime = t.get();
    double length = curtime - data.curtime;
    data.curtime = t.getSecond();
    data.freq = (m_nevts - data.nevts) / length / 1000. ;
    data.evtsize = m_datasize / (m_nevts - data.nevts);
    data.datasize += m_datasize;
    data.rate = m_datasize / length / 1000000.;
    B2INFO("Count = " << m_nevts << ", Freq = " << data.freq << " [kHz], "
           << "Rate = " << data.rate << " [MB/s], DataSize = "
           << m_datasize / 10000. / 1000 << " [kB/event]");
    data.evtno = evtmetadata->getEvent();
    data.nevts = m_nevts;
    RunInfoBuffer* info = StorageDeserializerModule::getInfo();
    if (info != NULL) {
      storager_data* p_data = (storager_data*)info->getReserved();
      info->lock();
      memcpy(p_data, &data, sizeof(data));
      info->notify();
      info->unlock();
    }
    m_datasize = 0;
  }
}

void StorageOutputModule::endRun()
{
  //fill Run data

  // End time
  storager_data& data(MonitorStorageModule::getData());
  double etime = (Time().get() - data.starttime) * 1000000;

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double avesize = m_size / (double)m_nevts;
  double avesize2 = m_size2 / (double)m_nevts;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  B2INFO("StorageOutput :  " << m_nevts << " events written with total bytes of " << m_size <<
         " kB, flow rate = " << flowmb << " (MB/s) event size = " << avesize << " +- " << sigma << " (kB)");
}


void StorageOutputModule::terminate()
{
  delete m_msghandler;
  delete m_streamer;
  delete m_file;

  B2INFO("terminate called")
}

SeqFile* StorageOutputModule::openDataFile()
{
  // StoreObjPtr<EventMetaData> evtmetadata;
  // int expno = evtmetadata->getExperiment();
  // int runno = evtmetadata->getRun();
  char outfile[1024];
  sprintf(outfile, "%s/e%4.4dr%6.6d.sroot",
          m_stordir.c_str(), m_expno, m_runno);
  SeqFile* seqfile = new SeqFile(outfile, "w");
  printf("StorageOutput : data file %s initialized\n", outfile);
  return seqfile;
}
