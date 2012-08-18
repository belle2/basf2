//+
// File : pseqrootoutput.cc
// Description : Sequential ROOT output module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/datastore/DataStore.h>
#include <framework/modules/seqroot/seqrootoutputModule.h>
#include <framework/core/Environment.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

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

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "SeqRoot file name.", string("SeqRootOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 0);

  B2DEBUG(1, "SeqRootOutput: Constructor done.");
}


SeqRootOutputModule::~SeqRootOutputModule()
{
}

void SeqRootOutputModule::initialize()
{

  // Open output file
  m_file = new SeqFile(m_outputFileName.c_str(), "w");
  //  m_fd = open ( m_outputFileName.c_str(), O_CREAT|O_WRONLY, 0644 );

  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

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
  EvtMessage* msg = buildMessage(MSG_EVENT);
  //  printf ( "SeqRootOutput : MsgSize = %d\n", msg->size() );

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());
  //  printf("SeqRootOuput : write = %d\n", stat);

  //  int sizebuf = htonl(msg->size());
  //  unsigned long sizebuf = (unsigned int)msg->size();
  //  int is = write ( m_fd, &sizebuf, 4 );
  //  is = write ( m_fd, msg->buffer(), msg->size() );
  //  printf ( "written bytes = %d\n", is );

  // Try to decode the buffer for debugging
  /*
  DataStore::Instance().clearMaps();
  vector<TObject*> objlist;
  vector<string> namelist;
  m_msghandler->clear();
  m_msghandler->decode_msg ( msg, objlist, namelist );
  */

  // Statistics
  double dsize = (double)stat / 1000.0;
  //  if ( dsize>100.0 && dsize < 800.0 ) {
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;
  //    printf ( "dsize = %f, dsize*dsize = %f; m_size=%f, m_size2=%f\n",
  //       dsize, dsize*dsize, m_size, m_size2 );
  //  }


  //  B2INFO ( "Event sent : " << m_nsent++ )
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
  printf("SeqRootOutput :  %d events read with total bytes of %f kB\n",
         m_nevt, m_size);
  printf("SeqRootOutput : flow rate = %f (MB/s)\n", flowmb);
  printf("SeqRootOutput : event size = %f +- %f (kB)\n", avesize, sigma);


  B2INFO("SeqRootOutput: endRun done.");
}


void SeqRootOutputModule::terminate()
{
  delete m_file;
  //  close ( m_fd );

  B2INFO("terminate called")
}

// Fill Datastore

EvtMessage* SeqRootOutputModule::buildMessage(RECORD_TYPE rectype)
{

  m_msghandler->clear();

  DataStore::EDurability durability = DataStore::c_Event;
  if (rectype == MSG_BEGIN_RUN)
    durability = DataStore::c_Run;

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
