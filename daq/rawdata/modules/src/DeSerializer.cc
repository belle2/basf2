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

  vector<int> default_port;
  default_port.push_back(1111);
  addParam("Port", m_port, "Receiver Ports (list)", default_port);
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

  // Open receiver sockets
  for (unsigned int i = 0; i < m_port.size(); i++) {
    m_recv.push_back(new EvtSocketRecv(m_port[i]));
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  // Initialize Array of RawCOPPER
  StoreArray<RawCOPPER>::registerPersistent();

  B2INFO("Rx initialized.");
}


void DeSerializerModule::beginRun()
{
  B2INFO("beginRun called.");
}

void DeSerializerModule::event()
{
  // DataStore interface
  StoreArray<RawCOPPER> rawcprary;

  // Read sockets and fill RawCOPPER object
  for (unsigned int i = 0; i < m_recv.size(); i++) {
    // Get a record from socket
    int stat = m_recv[i]->recv_buffer((char*)m_buffer);
    if (stat <= 0) {
      return; // Exit if EoD is found without updating EventMetaData
    }
    B2INFO("DeSer: port = " << m_port[i] << " stat = " << stat <<
           " : size=" << m_buffer[0]
           << " hdr = " << m_buffer[1] <<  " data = " << m_buffer[2]);
    //  int nw = m_buffer[0];

    // Fill RawCOPPER
    RawCOPPER rawcpr(m_buffer);

    // Put it in DataStore
    rawcprary.appendNew(rawcpr);
  }

  // Update EventMetaData
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nsent);

  m_nsent++;

  return;
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

