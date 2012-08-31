//+
// File : Rbuf2DsModule.cc
// Description : Module to restore DataStore from RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/Rbuf2DsModule.h>
#include <TSystem.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Rbuf2Ds)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Rbuf2DsModule::Rbuf2DsModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("InputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


Rbuf2DsModule::~Rbuf2DsModule()
{
}

void Rbuf2DsModule::initialize()
{
  gSystem->Load("libdataobjects");

  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Rx initialized.");
}


void Rbuf2DsModule::beginRun()
{
  B2INFO("beginRun called.");
}


void Rbuf2DsModule::event()
{
  // Get a record from ringbuf
  int size;

  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    usleep(100);
  }

  B2INFO("Rbuf2Ds: got an event from RingBuffer, size=" << size <<
         " (proc= " << (int)getpid() << ")");

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
  DataStore::EDurability durability = (DataStore::EDurability)(msg->header())->reserved[0];
  int nobjs = (msg->header())->reserved[1];
  int narrays = (msg->header())->reserved[2];

  B2INFO("Rbuf2Ds: nobjs = " << nobjs << ", narrays = " << narrays <<
         "(pid=" << (int)getpid() << ")");

  delete[] evtbuf;

  //  printf ( "Rx : Restoring objects\n" );

  // Restore objects in DataStore
  for (int i = 0; i < nobjs; i++) {
    if (objlist.at(i) != NULL) {
      DataStore::Instance().createEntry(string(namelist.at(i)), durability,
                                        (objlist.at(i))->Class(),
                                        false, true, false);
      DataStore::Instance().createObject(objlist.at(i), false,
                                         namelist.at(i), DataStore::c_Event,
                                         objlist.at(i)->IsA(), false);
      B2INFO("Rbuf2Ds: restored obj " << namelist.at(i));
    } else {
      B2INFO("Rbuf2Ds: obj " << namelist.at(i) << " is Null. Omitted");
    }
  }
  B2INFO("Rbuf2Ds: Objs restored");

  // Restore arrays in DataStore
  for (int i = 0; i < narrays; i++) {
    if (objlist.at(i + nobjs) != NULL) {
      DataStore::Instance().createEntry(string(namelist.at(i + nobjs)), durability,
                                        (objlist.at(i + nobjs))->Class(),
                                        true, true, false);
      DataStore::Instance().createObject(objlist.at(i + nobjs), false,
                                         namelist.at(i + nobjs), DataStore::c_Event,
                                         ((TClonesArray*)objlist.at(i + nobjs))->GetClass(), true);
      B2INFO("Rbuf2Ds: restored array " << namelist.at(i + nobjs));
    } else {
      B2INFO("Rbuf2Ds: array " << namelist.at(i + nobjs) << " is Null. Omitted");
    }

  }
  B2INFO("Rbuf2Ds: DataStore Restored!!");

  delete msg;

  return;
  //  return type;
}

void Rbuf2DsModule::endRun()
{
  //fill Run data

  B2INFO("Rbuf2Ds: endRun done.");
}


void Rbuf2DsModule::terminate()
{
  B2INFO("Rbuf2Ds: terminate called")
}

