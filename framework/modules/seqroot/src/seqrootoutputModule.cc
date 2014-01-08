//+
// File : pseqrootoutput.cc
// Description : Sequential ROOT output module for pbasf2
//
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
  setDescription("SeqROOT output module");
  //  setPropertyFlags(c_Output | c_ParallelProcessingCertified);
  m_file = 0;
  m_msghandler = 0;
  m_streamer = 0;

  vector<string> emptyvector;
  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "SeqRoot file name.", string("SeqRootOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticable effect. NOTE: Because of a ROOT bug, enabling this currently causes memory leaks..", 0);
  addParam("saveObjs", m_saveobjs, "List of objects to be saved", emptyvector);

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
  m_streamer->registerStreamObjs(m_saveobjs);

  B2INFO("SeqRootOutput: initialized.");

}


void SeqRootOutputModule::beginRun()
{
  //  EvtMessage* msg = buildMessage(MSG_BEGIN_RUN);

  //  m_file->write(msg->buffer());

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
  // - Choice of two methods
  //    Default is to use DataStoreStreamer class
  //    buildMessage function is still kept for further testing purpose
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);
  //  EvtMessage* msg = buildMessage(MSG_EVENT);

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

// Fill Datastore

EvtMessage* SeqRootOutputModule::buildMessage(RECORD_TYPE rectype)
{

  m_msghandler->clear();

  DataStore::EDurability durability = DataStore::c_Event;
  /*
  if (rectype == MSG_BEGIN_RUN)
    durability = DataStore::c_Run;
    */

  // Collect objects and place them in msghandler

  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(durability);
  int narrays = 0;
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = map.begin(); it != map.end(); ++it) {
    if (m_msghandler->add(it->second->ptr, it->first)) {
      B2INFO("Tx: adding item " << it->first);

      if (it->second->isArray)
        narrays++;
      else
        nobjs++;
    }
  }

  // Encode EvtMessage
  EvtMessage* msg = m_msghandler->encode_msg(rectype);
  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  //  printf ( "RecType = %d MsgSize = %d\n", rectype, msg->size() );
  return msg;

}
