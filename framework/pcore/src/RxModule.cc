//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/RxModule.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <TSystem.h>

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



RxModule::~RxModule() { }

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
  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    usleep(100);
  }
  B2INFO("Rx initialization: got an event from RingBuffer, size=" << size);

  // Restore objects in DataStore
  EvtMessage* evtmsg = new EvtMessage(evtbuf);
  if (evtmsg->type() == MSG_TERMINATE) {
    B2INFO("Rx initialization: got termination message. Exitting...");
  } else {
    m_streamer->restoreDataStore(evtmsg);
  }

  // Delete buffers
  delete evtmsg;
  delete[] evtbuf;

  m_nrecv = -1;
}


void RxModule::beginRun()
{
  B2DEBUG(100, "beginRun called.");
}


void RxModule::event()
{
  m_nrecv++;
  // First event is already loaded in initialize()
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  int size;
  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    usleep(100);
  }
  B2DEBUG(100, "Rx: got an event from RingBuffer, size=" << size);

  // Restore EvtMessage
  EvtMessage* msg = new EvtMessage(evtbuf);    // Have EvtMessage by ptr cpy
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting...");
  } else {
    m_streamer->restoreDataStore(msg);
    B2DEBUG(100, "Rx: DataStore Restored!");
  }

  // Remove buffers
  delete[] evtbuf;
  delete msg;
}

void RxModule::endRun()
{
  B2DEBUG(100, "endRun done.");
}


void RxModule::terminate()
{
  B2INFO("Rx: terminate called")
  delete m_streamer;
}
