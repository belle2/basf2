//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/RxModule.h>
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

RxModule::RxModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}

RxModule::RxModule(RingBuffer* rbuf) : Module()
{
  //Set module properties
  setDescription("Decode DataStore from RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  std::ostringstream buf; buf << "Rx" << rbuf->shmid();
  setModuleName(buf.str());

  m_rbuf = rbuf;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor with RingBuffer done.");
}



RxModule::~RxModule()
{
}

void RxModule::initialize()
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

  B2INFO("Rx initialized.");
}


void RxModule::beginRun()
{
  B2INFO("beginRun called.");
}


void RxModule::event()
{
  // Get a record from ringbuf
  int size;

  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    usleep(100);
  }

  B2INFO("Rx: got an event from RingBuffer, size=" << size);

  // Build EvtMessage and decompose it
  vector<TObject*> objlist;
  vector<string> namelist;
  EvtMessage* msg = new EvtMessage(evtbuf);    // Have EvtMessage by ptr cpy
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting....");
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }
  m_msghandler->decode_msg(msg, objlist, namelist);
  B2INFO("Rx: message decoded!");

  // Get Object info
  //  RECORD_TYPE type = msg->type();
  //  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = (msg->header())->reserved[1];
  int narrays = (msg->header())->reserved[2];

  //  printf ( "Rx: nobjs = %d, narrays = %d\n", nobjs, narrays );

  delete[] evtbuf;

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

void RxModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void RxModule::terminate()
{
  B2INFO("terminate called")
}

