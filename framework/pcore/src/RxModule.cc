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
  setPropertyFlags(c_Input | c_InternalSerializer);

  m_rbuf = rbuf;
  m_compressionLevel = 0;
  if (rbuf) {
    std::ostringstream buf; buf << "Rx" << rbuf->shmid();
    setModuleName(buf.str());
    B2INFO("Rx: Constructor with RingBuffer done.");
  }
}



RxModule::~RxModule() { }

void RxModule::readEvent()
{
  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  while (m_rbuf->continueReadingData()) {
    int size = m_rbuf->remq((int*)evtbuf);
    if (size != 0) {
      B2DEBUG(100, "Rx: got an event from RingBuffer, size=" << size);

      // Restore objects in DataStore
      EvtMessage evtmsg(evtbuf);
      m_streamer->restoreDataStore(&evtmsg);
      break;
    }
    usleep(20);
  }

  delete[] evtbuf;
}

void RxModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).
  readEvent();
}

void RxModule::beginRun() { }

void RxModule::event()
{
  m_nrecv++;
  // First event is already loaded in initialize()
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  readEvent();
}

void RxModule::endRun() { }

void RxModule::terminate()
{
  B2INFO("Rx: terminate called")
  delete m_streamer;
}
