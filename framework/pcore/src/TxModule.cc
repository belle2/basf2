//+
// File : TxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/TxModule.h>
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

TxModule::TxModule() : Module(), m_msghandler(0), m_streamer(0)
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2DEBUG(1, "Tx: Constructor done.");
}

TxModule::TxModule(RingBuffer* rbuf) : Module(), m_msghandler(0), m_streamer(0)
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_Input | c_InitializeInProcess);
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  std::ostringstream buf; buf << "Tx" << rbuf->shmid();
  setModuleName(buf.str());

  m_rbuf = rbuf;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Tx: Constructor with RingBuffer done.");
}



TxModule::~TxModule()
{
  delete m_streamer;
  //  delete m_msghandler;
}

void TxModule::initialize()
{
  //  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  B2INFO(getName() << " initialized.");
}


void TxModule::beginRun()
{
  B2INFO("beginRun called.");
}


void TxModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    usleep(200);
  }
  m_nsent++;

  B2INFO("Tx: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

}

void TxModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void TxModule::terminate()
{
  B2INFO("terminate called")
}

