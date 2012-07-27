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

TxModule::TxModule() : Module(), m_msghandler(0)
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

TxModule::TxModule(RingBuffer* rbuf) : Module(), m_msghandler(0)
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
  delete m_msghandler;
}

void TxModule::initialize()
{
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
  const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(durability);
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = map.begin(); it != map.end(); ++it) {
    //    if ( it->second != NULL ) {
    if (dynamic_cast<TClonesArray*>(it->second))
      continue;
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("Tx: adding obj " << it->first);
      nobjs++;
    }
    //    }
  }
  // Stream arrays in msg_handler
  int narrays = 0;
  for (DataStore::StoreObjConstIter it = map.begin(); it != map.end(); ++it) {
    //    if ( it->second != NULL ) {
    if (dynamic_cast<TClonesArray*>(it->second) == 0)
      continue;
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("Tx: adding array " << it->first);
      narrays++;
    }
    //    }
  }

  //  printf ( "Tx: nobjs = %d, narrays = %d (pid=%d)\n", nobjs, narrays, (int)getpid() );
  B2INFO("Tx: nobjs = " << nobjs << ", narrays = " << narrays <<
         " (pid=" << (int)getpid() << ")");

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

  B2INFO("Tx: objs sent in buffer. Size = " << msg->size());

  // Try to decode the buffer for debugging
  //  vector<TObject*> objlist;
  //  vector<string> namelist;
  //  m_msghandler->decode_msg ( msg, objlist, namelist );

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

