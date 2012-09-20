//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/RxModule.h>

#include <TSystem.h>

#include <stdlib.h>

#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Rx)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RxModule::RxModule(RingBuffer* rbuf) : Module(), m_streamer(0), m_nrecv(-1)
{
  //Set module properties
  setDescription("Decode data from RingBuffer into DataStore");
  setPropertyFlags(c_Input | c_InitializeInProcess);

  m_rbuf = rbuf;
  m_compressionLevel = 0;
  if (rbuf) {
    std::ostringstream buf; buf << "Rx" << rbuf->shmid();
    setModuleName(buf.str());
    B2INFO("Rx: Constructor with RingBuffer done.");
  }
}



RxModule::~RxModule()
{
  //TODO: why isn't this cleaned up?
  //  delete m_streamer;
}

void RxModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);


  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).

  // Prefetch the first record in Ring Buffer
  int size;
  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    usleep(100);
  }
  B2INFO("Rx initialization: got an event from RingBuffer, size=" << size);

  // Restore objects in DataStore
  EvtMessage* evtmsg = new EvtMessage(evtbuf);
  m_streamer->restoreDataStore(evtmsg);

  // Delete buffers
  delete evtmsg;
  delete[] evtbuf;

  m_nrecv = -1;
}


void RxModule::beginRun()
{
  B2INFO("beginRun called.");
}


void RxModule::event()
{
  m_nrecv++;
  // First event is already loaded in initialize()
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  int size;
  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    usleep(100);
  }
  B2INFO("Rx: got an event from RingBuffer, size=" << size <<
         " (proc= " << (int)getpid() << ")");

  // Restore EvtMessage
  EvtMessage* msg = new EvtMessage(evtbuf);    // Have EvtMessage by ptr cpy
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting....");
    delete[] evtbuf;
    delete msg;
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Restore DataStore
  m_streamer->restoreDataStore(msg);

  // Remove buffers
  delete[] evtbuf;
  delete msg;

  B2INFO("Rx: DataStore Restored!!");

  return;
}

void RxModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void RxModule::terminate()
{
  B2INFO("terminate called")
}
