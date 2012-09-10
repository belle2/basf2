//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rawdata/modules/DeSerializer.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerModule::DeSerializerModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Port", m_port, "Receiver Port", 1111);
  m_nsent = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


DeSerializerModule::~DeSerializerModule()
{
  delete[] m_buffer;
}

void DeSerializerModule::initialize()
{

  // Open receiving socekt
  m_recv = new EvtSocketRecv(m_port);

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EventMetaData
  StoreObjPtr<EventMetaData>::registerPersistent();

  B2INFO("Rx initialized.");
}


void DeSerializerModule::beginRun()
{
  B2INFO("beginRun called.");
}


void DeSerializerModule::event()
{
  // Event Meta Data
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  eventMetaDataPtr->setExperiment(1);
  eventMetaDataPtr->setRun(1);
  eventMetaDataPtr->setEvent(m_nsent);

  // Get a record from socket
  int stat = m_recv->recv_buffer((char*)m_buffer);
  if (stat <= 0) {
    return;
  }
  B2INFO("DeSer: stat = " << stat << " : size=" << m_buffer[0]
         << " hdr = " << m_buffer[1] <<  " data = " << m_buffer[2]);
  /*
  if (msg->type() == MSG_TERMINATE){
    B2INFO ( "Rx: got termination message. Exitting...." );
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Build EvtMessage and decompose it
  vector<TObject*> objlist;
  vector<string> namelist;
  m_msghandler->decode_msg(msg, objlist, namelist);
  B2INFO ( "Rx: message decoded!" );

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
    B2INFO ( "Rx: restored obj " << namelist.at(i) );
  }
  B2INFO ( "Rx: Objs restored" );

  // Restore arrays in DataStore
  for (int i = 0; i < narrays; i++) {
    DataStore::Instance().storeArray((TClonesArray*)objlist.at(i + nobjs),
                                     namelist.at(i+nobjs));
    B2INFO ( "Rx: restored array " << namelist.at(i+nobjs) );
  }
  B2INFO ( "Rx: DataStore Restored!!" );

  delete msg;
  */

  m_nsent++;

  //  if ( m_nsent%1000 == 0 ) printf ( "event = %d\n", m_nsent );
  //  printf ( "event = %d\n", m_nsent );


  return;
  //  return type;
}

void DeSerializerModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void DeSerializerModule::terminate()
{
  B2INFO("terminate called")
}

