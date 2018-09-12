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
#include <framework/pcore/ProcHandler.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/Environment.h>

#include <stdlib.h>

using namespace std;
using namespace Belle2;

TxModule::TxModule(RingBuffer* rbuf) : Module(), m_streamer(nullptr), m_blockingInsert(true)
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_Input | c_InternalSerializer);
  setType("Tx");

  m_rbuf = rbuf;
  m_nsent = 0;
  m_compressionLevel = 0;

  if (rbuf) {
    setName("Tx" + std::to_string(rbuf->shmid()));
    B2DEBUG(32, "Tx: Constructor with RingBuffer done.");
  }
}

TxModule::~TxModule() { }

void TxModule::initialize()
{
  if (m_sendRandomState)
    m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);

  m_rbuf->txAttached();
  m_streamer = new DataStoreStreamer(m_compressionLevel, m_handleMergeable);

  if ((Environment::Instance().getStreamingObjects()).size() > 0) {
    m_streamer->setStreamingObjects(Environment::Instance().getStreamingObjects());
    B2DEBUG(32, "Tx: Streaming objects limited : " << (Environment::Instance().getStreamingObjects()).size() << " objects");
  }

  B2DEBUG(32, getName() << " initialized.");
}


void TxModule::beginRun()
{
  if (ProcHandler::isInputProcess()) {
    //NOTE: only needs to be done in input process, that way the parallel processes
    //      will never see runs out of order
    B2DEBUG(35, "beginRun called (will wait for reading processes to finish processing previous run...).");
    //wait until RB is both empty and all attached reading processes have finished..
    while (!m_rbuf->isDead() and !m_rbuf->allRxWaiting()) {
      usleep(500);
    }
  }
  B2DEBUG(35,  "beginRun done.");
}


void TxModule::event()
{
  if (m_sendRandomState) {
    //Save event level random generator into datastore to send it to other processes
    if (!m_randomgenerator.isValid()) {
      m_randomgenerator.construct(RandomNumbers::getEventRandomGenerator());
    } else {
      *m_randomgenerator = RandomNumbers::getEventRandomGenerator();
    }
  }

  // Stream DataStore in EvtMessage, also stream transient objects and objects of durability c_Persistent
  EvtMessage* msg = m_streamer->streamDataStore(true, true);

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

  B2DEBUG(35, "Tx: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;
}

void TxModule::endRun()
{
  B2DEBUG(35, "endRun done.");
}


void TxModule::terminate()
{
  B2DEBUG(32, "Tx: terminate called");

  m_rbuf->txDetached();
  delete m_streamer;
  m_rbuf = nullptr;
}

