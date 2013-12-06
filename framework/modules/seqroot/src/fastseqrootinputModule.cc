//+
// File : fastseqrootinput.cc
// Description : Sequential ROOT input module for pbasf2 (fast object destreaming)
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012     modified to use DataStoreStreamer, clean up
//        29 - Nov - 2013     object destreaming in threads
//-

#include <framework/modules/seqroot/fastseqrootinputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TSystem.h>

#include <cmath>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;
using namespace Belle2;

static FastSeqRootInputModule* s_input = NULL;

//-----------------------------------------------------------------
// File Read Thread Interface
//-----------------------------------------------------------------
void* RunFileReader(void*)
{
  s_input->ReadFileInThread();
  return NULL;
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FastSeqRootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FastSeqRootInputModule::FastSeqRootInputModule() : Module()
{
  //Set module properties
  setDescription("SeqROOT input module");
  setPropertyFlags(c_Input);

  m_file = 0;
  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "FastSeqRoot file name.", string("FastSeqRootInput.root"));

  s_input = this;

  B2DEBUG(1, "FastSeqRootInput: Constructor done.");
}


FastSeqRootInputModule::~FastSeqRootInputModule()
{
}

void FastSeqRootInputModule::initialize()
{
  gSystem->Load("libdataobjects");

  const std::vector<std::string>& inputFiles = Environment::Instance().getInputFilesOverride();
  if (!inputFiles.empty()) {
    if (inputFiles.size() > 1) {
      B2FATAL("FastSeqRootInput only accepts a single input file.")
      return;
    }
    m_inputFileName = inputFiles[0];
  }

  // Initialize DataStoreStreamer, use Instance to use threads
  m_streamer = &(DataStoreStreamer::Instance());
  m_streamer->setMaxThreads(4);

  // Read the first event in FastSeqRoot file and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).
  EvtMessage* evtmsg = NULL;
  // Open input file
  m_file = new SeqFile(m_inputFileName.c_str(), "r");
  if (m_file->status() <= 0)
    B2FATAL("FastSeqRootInput : Error in opening input file : " << m_inputFileName);
  int size;
  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  evtmsg = new EvtMessage(evtbuf);

  // Read 1st event in DataStore
  size = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
  if (size > 0) {
    evtmsg = new EvtMessage(evtbuf);
    m_streamer->restoreDataStore(evtmsg);
  } else {
    B2FATAL("SeqRootInput : Error in reading first event")
  }
  delete evtmsg;
  delete[] evtbuf;

  // Create a new sustainable thread to read events and queue them
  // to decoder
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  //  pthread_attr_setschedpolicy(&thread_attr , SCHED_FIFO);
  //  pthread_attr_setdetachstate(&thread_attr , PTHREAD_CREATE_DETACHED);
  //  pthread_t thr_input;
  pthread_create(&m_thr_input, NULL, RunFileReader, NULL);

  B2INFO("FastSeqRootInput: initialized.");
}

void FastSeqRootInputModule::ReadFileInThread()
{
  printf("ReadFileInThread started!!\n");
  int rf_nevt = 0;
  for (;;) {
    char* evtbuf = new char[EvtMessage::c_MaxEventSize];
    int size = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (size == 0) {
      printf("ReadFileInThread : EOF detected!!!!!\n");
      m_streamer->queueEvtMessage(NULL);
      delete m_file;
      m_file = 0;
      delete[] evtbuf;
      return;
    } else if (size > 0) {
      m_streamer->queueEvtMessage(evtbuf);
    } else {
      B2FATAL("FastSeqRootInput : Error in reading first event");
    }
    // Statistics
    double dsize = (double)size / 1000.0;
    m_size += dsize;
    m_size2 += dsize * dsize;
    //    printf ( "ReadFileInThread : event read, size=%d\n", size );
    rf_nevt++;
    //    if ( rf_nevt%1000 == 0 ) printf ( "ReadFileInThread : %d events\n", rf_nevt );
  }
}


void FastSeqRootInputModule::beginRun()
{
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;
  B2INFO("FastSeqRootInput: beginRun called.");
}


void FastSeqRootInputModule::event()
{
  m_nevt++;

  // First event is already loaded
  if (m_nevt == 0) return;

  //  printf ( "FastSeqRootInput : event = %d, start processing\n", m_nevt );

  // Restore DataStore
  m_streamer->restoreDataStoreAsync();

}

void FastSeqRootInputModule::endRun()
{
  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double evrate = (double)m_nevt / (etime / 1000.0);
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  B2INFO("FastSeqRootInput :  " << m_nevt << " events read with total bytes of " << m_size << " kB");
  B2INFO("FastSeqRootInput : event rate = " << evrate << " (KHz)");
  B2INFO("FastSeqRootInput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("FastSeqRootInput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("FastSeqRootInput: endRun done.");
}


void FastSeqRootInputModule::terminate()
{
  pthread_join(m_thr_input, NULL);
  //  delete m_streamer;
  //  delete m_file;
  B2INFO("FastSeqRootInput: terminate called")
}

