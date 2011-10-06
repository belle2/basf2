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

TxModule::TxModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Tx: Constructor done.");
}

TxModule::TxModule(RingBuffer* rbuf) : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
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
}

void TxModule::initialize()
{

  // get iterators
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    m_obj_iter[ii]   = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(ii));
    m_array_iter[ii] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(ii));
    //    m_done[ii]     = false;
  }

  // Attach to ring buffer if nprocess > 0
  //  m_nproc = Framework::nprocess();
  //  B2WARNING("TX : nproc = " << m_nproc)

  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Tx initialized.");
}


void TxModule::beginRun()
{
  B2INFO("beginRun called.");
}


void TxModule::event()
{
  // Clear msghandler
  m_msghandler->clear();

  // Set durability
  DataStore::EDurability durability = DataStore::c_Event;

  // Stream objects in msg_handler
  m_obj_iter[durability]->first();
  int nobjs = 0;
  while (!m_obj_iter[durability]->isDone()) {
    B2INFO("Tx: adding obj " << m_obj_iter[durability]->key());
    m_msghandler->add(m_obj_iter[durability]->value(), m_obj_iter[durability]->key());
    nobjs++;
    m_obj_iter[durability]->next();
  }
  // Stream arrays in msg_handler
  m_array_iter[durability]->first();
  int narrays = 0;
  while (!m_array_iter[durability]->isDone()) {
    TClonesArray* ary = (TClonesArray*)m_array_iter[durability]->value();
    B2INFO("Tx: adding array " << m_array_iter[durability]->key() << " size = " << ary->GetEntries());
    m_msghandler->add(m_array_iter[durability]->value(), m_array_iter[durability]->key());
    narrays++;
    m_array_iter[durability]->next();
  }

  //  printf ( "Tx: nobjs = %d, narrays = %d\n", nobjs, narrays );

  // Encode event message
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  // Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    usleep(200);
  }

  // Release EvtMessage buffer
  delete msg;

  B2INFO("Tx: objs sent in buffer. Size = " << msg->size());
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

