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
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticable effect. NOTE: Because of a ROOT bug, enabling this currently causes memory leaks..", 0);
  addParam("OutputBufferName", m_obufname, "Name of Ring Buffer to dump streamed events",
           string(""));
  addParam("DumpInterval", m_interval, "Event interval to dump event in RingBuffer",
           10);
  addParam("StorageDir", m_stordir, "Directory to write output files", string(""));

  B2DEBUG(1, "StorageOutput: Constructor done.");
}


StorageOutputModule::~StorageOutputModule() { }

void StorageOutputModule::initialize()
{
  // Open output file
  /*
  m_file = new SeqFile(m_outputFileName.c_str(), "w");
  if ( m_file == 0 )
    B2FATAL ( "Error to open output file" );
  */

  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  // DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  // Ring Buffer
  if (m_obufname.size() > 0) {
    B2INFO(m_obufname.c_str());;
    m_obuf = new RingBuffer(m_obufname.c_str(), 10000000);
  } else
    m_obuf = NULL;

  B2INFO("StorageOutput: initialized.");

}


void StorageOutputModule::beginRun()
{
  // Statistics
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;

  // Open data file by looking at exp/run number
  if (m_file != NULL) delete m_file;
  m_file = openDataFile();

  B2INFO("StorageOutput: beginRun called.");
}

void StorageOutputModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());
  //  printf("StorageOuput : write = %d\n", stat);

  //static int count = 0;
  // Queue EvtMessage in RingBuffer
  if (m_obuf != NULL) {
    if (m_nevt % m_interval == 0) {
      // Free running RingBuffer, no flow control
      //count++;
      m_obuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
      //m_obuf->insq((int*)msg->buffer(), *(int*)msg->buffer());
    }
  }
  // Clean up EvtMessage
  delete msg;

  // Statistics
  double dsize = (double)stat / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;
}

void StorageOutputModule::endRun()
{
  //fill Run data

  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  //  printf ( "m_size = %f, m_size2 = %f, m_nevt = %d\n", m_size, m_size2, m_nevt );
  //  printf ( "avesize2 = %f, avesize = %f, avesize*avesize = %f\n", avesize2, avesize, avesize*avesize );
  B2INFO("StorageOutput :  " << m_nevt << " events written with total bytes of " << m_size << " kB");
  B2INFO("StorageOutput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("StorageOutput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("StorageOutput: endRun done.");
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
  StoreObjPtr<EventMetaData> evtmetadata;
  int expno = evtmetadata->getExperiment();
  int runno = evtmetadata->getRun();
  char outfile[1024];
  sprintf(outfile, "%s/e%4.4dr%6.6d.sroot",
          m_stordir.c_str(), expno, runno);
  SeqFile* seqfile = new SeqFile(outfile, "w");
  printf("StorageOutput : data file %s initialized\n", outfile);
  return seqfile;
}
