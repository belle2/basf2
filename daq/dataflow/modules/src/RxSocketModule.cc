//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/dataflow/modules/RxSocketModule.h>
#include <stdlib.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RxSocket)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RxSocketModule::RxSocketModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Port", m_port, "Receiver Port", 1111);
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


RxSocketModule::~RxSocketModule()
{
}

void RxSocketModule::initialize()
{

  // Open receiving socekt
  m_recv = new EvtSocketRecv(m_port);

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Rx initialized.");
}


void RxSocketModule::beginRun()
{
  B2INFO("beginRun called.");
}


void RxSocketModule::event()
{
  // Get a record from socket
  EvtMessage* msg = m_recv->recv();
  if (msg == NULL) {
    return;
  }
  B2INFO("Rx: got an event from Socket, size=" << msg->size());
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting....");
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Build EvtMessage and decompose it
  vector<TObject*> objlist;
  vector<string> namelist;
  m_msghandler->decode_msg(msg, objlist, namelist);
  B2INFO("Rx: message decoded!");

  // Get Object info
  RECORD_TYPE type = msg->type();
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = (msg->header())->reserved[1];
  int narrays = (msg->header())->reserved[2];

  //  printf ( "Rx: nobjs = %d, narrays = %d\n", nobjs, narrays );

  // Restore objects in DataStore
  for (int i = 0; i < nobjs; i++) {
    DataStore::Instance().storeObject(objlist.at(i),
                                      namelist.at(i));
    B2INFO("Rx: restored obj " << namelist.at(i));
  }
  B2INFO("Rx: Objs restored");

  // Restore arrays in DataStore
  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs),
                                     namelist.at(i + nobjs));
    B2INFO("Rx: restored array " << namelist.at(i + nobjs));
  }
  B2INFO("Rx: DataStore Restored!!");

  delete msg;

  return;
  //  return type;
}

void RxSocketModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void RxSocketModule::terminate()
{
  B2INFO("terminate called")
}

