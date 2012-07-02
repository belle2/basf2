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

  B2INFO("Rx: got an event from RingBuffer, size=" << size <<
         " (proc= " << (int)getpid() << ")");
  //  printf ( "Rx: got an event from RingBuffer, size=%d (proc=%d)\n", size,
  //     (int)getpid() );
  //  fflush ( stdout );

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

  // Get Object info
  //  RECORD_TYPE type = msg->type();
  // DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = (msg->header())->reserved[1];
  int narrays = (msg->header())->reserved[2];

  B2INFO("Rx: nobjs = " << nobjs << ", narrays = " << narrays <<
         "(pid=" << (int)getpid() << ")");

  //  printf ( "Rx : nobjs = %d, narrays = %d, (size=%d)\n", nobjs, narrays,
  //     objlist.size() );

  delete[] evtbuf;

  //  printf ( "Rx : Restoring objects\n" );

  // Restore objects in DataStore
  for (int i = 0; i < nobjs; i++) {
    //    TObject* obj = dynamic_cast<namelist.at(i).c_str()>objlist.at(i) );
    if (objlist.at(i) != NULL) {
      DataStore::Instance().storeObject(objlist.at(i),
                                        namelist.at(i));
      //    printf ( "Rx: Restoring [Object] %s : Class=%s\n",
      //       namelist.at(i).c_str(), ((objlist.at(i))->ClassName()).c_str() );
      B2INFO("Rx: restored obj " << namelist.at(i));
    } else {
      B2INFO("Rx: obj " << namelist.at(i) << " is Null. Omitted");
    }
  }
  B2INFO("Rx: Objs restored");

  //  DataStore::Instance().clearMaps();

  // Restore arrays in DataStore
  for (int i = 0; i < narrays; i++) {
    //    printf ( "Rx : restoring index=%d\n", i+nobjs );
    //TClonesArray* adrs = (TClonesArray*) objlist.at(i + nobjs);
    //    printf ( "Rx: Address of %s = %8.8x (pid=%d)\n", (namelist.at(i+nobjs)).c_str(), adrs, (int)getpid() );
    //    printf ( "Rx: Restoring [Array] %s\n", namelist.at(i+nobjs).c_str() );

    //    fflush ( stdout );
    if (objlist.at(i + nobjs) != NULL) {
      DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs),
                                       namelist.at(i + nobjs));
      B2INFO("Rx: restored array " << namelist.at(i + nobjs));
    } else {
      B2INFO("Rx: array " << namelist.at(i + nobjs) << " is Null. Omitted");
    }

  }
  B2INFO("Rx: DataStore Restored!!");
  /*
  // Debug dump
  DataStore::StoreObjMap objmap = DataStore::Instance().getObjectMap(DataStore::c_Event);
  TObject* obj = objmap["MCParticlesToPXDSimHits"];
  printf ( "Rx: objmap size = %d, MCParticlesToPXDSimHits = %8.8x (pid=%d)\n", objmap.size(), obj, getpid() );
  //    if ( obj == NULL ) {
  for ( DataStore::StoreObjIter it=objmap.begin(); it!=objmap.end(); ++it ) {
    //      TObject* obj = *it;
    printf ( "objmap containts %s, adr=%8.8x\n", ((*it).first).c_str(), (*it).second );
  }
  */

  //  printf ( "Rx : Objects restored\n" );

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

