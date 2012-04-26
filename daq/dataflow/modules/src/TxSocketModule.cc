//+
// File : TxSocketModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/dataflow/modules/TxSocketModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TxSocket)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TxSocketModule::TxSocketModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("DestHostName", m_dest, "Destination host", string("localhost"));
  addParam("DestPort", m_port, "Destination port", 1111);

  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Tx: Constructor done.");
}



TxSocketModule::~TxSocketModule()
{
}

void TxSocketModule::initialize()
{

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port);

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Tx initialized.");
}


void TxSocketModule::beginRun()
{
  B2INFO("beginRun called.");
}


void TxSocketModule::event()
{
  // Clear msghandler
  m_msghandler->clear();

  // Set durability
  DataStore::EDurability durability = DataStore::c_Event;

  // Stream objects in msg_handler
  const DataStore::StoreObjMap& objmap = DataStore::Instance().getObjectMap(durability);
  int nobjs = 0;
  for (DataStore::StoreObjConstIter it = objmap.begin(); it != objmap.end(); ++it) {
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("TxSocket: adding obj " << it->first);
      nobjs++;
    }
  }
  // Stream arrays in msg_handler
  const DataStore::StoreArrayMap& arymap = DataStore::Instance().getArrayMap(durability);
  int narrays = 0;
  for (DataStore::StoreObjConstIter it = arymap.begin(); it != arymap.end(); ++it) {
    if (m_msghandler->add(it->second, it->first)) {
      B2INFO("TxSocket: adding array " << it->first);
      narrays++;
    }
  }
  B2INFO("TxSocket: nobjs = " << nobjs << ", narrays = " << narrays <<
         " (pid=" << (int)getpid() << ")");


  // Encode event message
  EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = (int)durability;
  (msg->header())->reserved[1] = nobjs;       // No. of objects
  (msg->header())->reserved[2] = narrays;    // No. of arrays

  // Send the message to Socket
  int stat = m_sock->send(msg);
  B2INFO("Tx: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

}

void TxSocketModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void TxSocketModule::terminate()
{
  B2INFO("terminate called")
}

