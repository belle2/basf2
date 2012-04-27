//+
// File : SerializerModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rawdata/modules/Serializer.h>

using namespace std;
using namespace Belle2;

extern RawCDC* RAWCDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Serializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SerializerModule::SerializerModule() : Module()
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



SerializerModule::~SerializerModule()
{
}

void SerializerModule::initialize()
{

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port);

  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  B2INFO("Tx initialized.");
}


void SerializerModule::beginRun()
{
  B2INFO("beginRun called.");
}


void SerializerModule::event()
{
  // Hand written streamer
  int* buf = RAWCDC->buffer();
  int nbytes = buf[0] * 4;
  //  nbytes = 4;

  int stat = m_sock->send_buffer(nbytes, (char*)buf);

  //  m_msghandler->add( &evm, "EventMetaData" );

}

void SerializerModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void SerializerModule::terminate()
{
  B2INFO("terminate called")
}

