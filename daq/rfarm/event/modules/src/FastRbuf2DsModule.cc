/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/FastRbuf2DsModule.h>
#include <TSystem.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

static FastRbuf2DsModule* s_input = NULL;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunRbufReader(void*)
{
  s_input->ReadRbufInThread();
  return NULL;
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FastRbuf2Ds)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FastRbuf2DsModule::FastRbuf2DsModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("InputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);
  addParam("NumThreads", m_numThread, "Number of threads for object decoding",
           1);

  m_rbuf = NULL;
  m_nrecv = 0;
  //  m_compressionLevel = 0;

  s_input = this;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


FastRbuf2DsModule::~FastRbuf2DsModule()
{
}

void FastRbuf2DsModule::initialize()
{
  gSystem->Load("libdataobjects");

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(m_rbufname.c_str());

  // Initialize DataStoreStreamer, use Instance to use threads
  //  m_streamer = &(DataStoreStreamer::Instance());
  m_streamer = new DataStoreStreamer(m_compressionLevel, true, m_numThread);

  // Read the first event in FastSeqRoot file and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).
  EvtMessage* evtmsg = NULL;

  // Prefetch the first record in Ring Buffer
  int size;
  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) usleep(20);

  // Read 1st event in DataStore
  if (size > 0) {
    evtmsg = new EvtMessage(evtbuf);
    m_streamer->restoreDataStore(evtmsg);
  } else {
    B2FATAL("SeqRootInput : Error in reading first event");
  }
  delete evtmsg;
  delete[] evtbuf;

  // Create decoder threads
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_thr_input, NULL, RunRbufReader, NULL);


  /*
  for (;;) {
    if ( m_streamer->getDecoderStatus() == 0 ) break;
    while ((size = m_rbuf->remq((int*)evtbuf)) == 0) usleep ( 20 );
    if (size > 0) {
      m_streamer->queueEvtMessage(evtbuf);
    } else {
      B2FATAL("FastSeqRootInput : Error in reading first event");
    }
  }
  */

  //  delete[] evtbuf;

  m_nrecv = -1;

  B2INFO("Rx initialized.");
}

void FastRbuf2DsModule::ReadRbufInThread()
{
  printf("ReadFileInThread started!!\n");
  int rf_nevt = 0;
  int size = 0;
  for (;;) {
    char* evtbuf = new char[EvtMessage::c_MaxEventSize];
    while ((size = m_rbuf->remq((int*)evtbuf)) == 0) usleep(20);
    if (size == 0) {
      printf("ReadRbufInThread : ERROR! record with size=0 detected!!!!!\n");
      m_streamer->queueEvtMessage(NULL);
      delete[] evtbuf;
      return;
    } else if (size > 0) {
      m_streamer->queueEvtMessage(evtbuf);
    } else {
      B2FATAL("FastRbuf2Ds : Error in reading first event");
    }
    rf_nevt++;
    //    if ( rf_nevt%1000 == 0 ) printf ( "ReadRbufInThread : %d events\n", rf_nevt );
  }
}


void FastRbuf2DsModule::beginRun()
{
  B2INFO("beginRun called.");
}


void FastRbuf2DsModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;

  // Restore DataStore with objects in top of queue
  m_streamer->restoreDataStoreAsync();

  B2INFO("FastRbuf2Ds: DataStore Restored!!");
  return;
  //  return type;
}

void FastRbuf2DsModule::endRun()
{
  //fill Run data

  B2INFO("FastRbuf2Ds: endRun done.");
}


void FastRbuf2DsModule::terminate()
{
  pthread_join(m_thr_input, NULL);
  B2INFO("FastRbuf2Ds: terminate called");
}

