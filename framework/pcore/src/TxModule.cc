//+
// File : TxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/TxModule.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Tx)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TxModule::TxModule(RingBuffer* rbuf) : Module(), m_streamer(0), m_blockingInsert(true)
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_Input | c_InternalSerializer);

  m_rbuf = rbuf;
  m_nsent = 0;
  m_compressionLevel = 0;

  if (rbuf) {
    std::ostringstream buf; buf << "Tx" << rbuf->shmid();
    setModuleName(buf.str());
    B2INFO("Tx: Constructor with RingBuffer done.");
  }
}

TxModule::~TxModule() { }

void TxModule::initialize()
{
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  B2INFO(getName() << " initialized.");
}


void TxModule::beginRun()
{
  B2DEBUG(100, "beginRun called.");
}


void TxModule::event()
{
  // Stream DataStore in EvtMessage, also stream transient objects
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event, true);

  // Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), msg->paddedSize());
    if (stat >= 0) break;
    if (!m_blockingInsert) {
      B2WARNING("Ring buffer seems full, removing some previous data.");
      m_rbuf->remq(NULL);
    }
    //    usleep(200);
    usleep(20);
  }
  m_nsent++;

  B2DEBUG(100, "Tx: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;
}

void TxModule::endRun()
{
  B2DEBUG(100, "endRun done.");
}


void TxModule::terminate()
{
  B2INFO("terminate called")

  delete m_streamer;
}

