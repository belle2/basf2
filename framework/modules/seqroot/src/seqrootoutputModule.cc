//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//-

#include <framework/modules/seqroot/seqrootoutputModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/core/Environment.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>
#include <fcntl.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootOutputModule::SeqRootOutputModule() : Module()
{
  //Set module properties
  setDescription("Module for sequential ROOT I/O. As an alternative to the TTree format, this format stores data as a sequential stream of objects plus a small meta-data header, which doesn't impose the overhead of the TTree and may result in higher read rates from hard disks. It is also the storage format used by DAQ and HLT. SeqRoot files however tend be a factor 2-3 larger than their .root equivalents.");
  m_file = 0;
  m_msghandler = 0;
  m_streamer = 0;

  vector<string> emptyvector;
  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "SeqRoot file name.", string("SeqRootOutput.sroot"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticable effect. NOTE: Because of a ROOT bug ( https://sft.its.cern.ch/jira/browse/ROOT-4550 ), enabling this currently causes memory leaks and is disabled.", 0);
  addParam("saveObjs", m_saveObjs, "List of objects/arrays to be saved", emptyvector);

  B2DEBUG(1, "SeqRootOutput: Constructor done.");
}


SeqRootOutputModule::~SeqRootOutputModule() { }

void SeqRootOutputModule::initialize()
{
  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_outputFileName = outputFileArgument;

  // Open output file
  m_file = new SeqFile(m_outputFileName.c_str(), "w");

  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  // DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_streamer->registerStreamObjs(m_saveObjs);

  B2INFO("SeqRootOutput: initialized.");

}


void SeqRootOutputModule::beginRun()
{

  // Statistics
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;

  B2INFO("SeqRootOutput: beginRun called.");
}

void SeqRootOutputModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());
  //  printf("SeqRootOuput : write = %d\n", stat);

  // Clean up EvtMessage
  delete msg;

  // Statistics
  double dsize = (double)stat / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;
}

void SeqRootOutputModule::endRun()
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
  B2INFO("SeqRootOutput :  " << m_nevt << " events written with total bytes of " << m_size << " kB");
  B2INFO("SeqRootOutput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("SeqRootOutput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("SeqRootOutput: endRun done.");
}


void SeqRootOutputModule::terminate()
{
  delete m_msghandler;
  delete m_streamer;
  delete m_file;

  B2INFO("terminate called")
}
