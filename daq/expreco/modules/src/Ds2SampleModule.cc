//+
// File : Ds2SampleModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/expreco/modules/Ds2SampleModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Ds2Sample)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Ds2SampleModule::Ds2SampleModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_InitializeInProcess);

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("OutputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Ds2Sample: Constructor done.");
}


Ds2SampleModule::~Ds2SampleModule()
{
}

void Ds2SampleModule::initialize()
{

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(m_rbufname.c_str());
  //  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  B2INFO("Ds2Sample initialized.");
}


void Ds2SampleModule::beginRun()
{
  B2INFO("Ds2Sample: beginRun called.");
}


void Ds2SampleModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Put the message in ring buffer. If failed, just skip the event
  int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);

  B2INFO("Ds2Sample: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

  // return
  m_nsent++;

}

void Ds2SampleModule::endRun()
{
  //fill Run data

  printf("Ds2Sample: endRun called.....\n");
  B2INFO("Ds2Sample: endRun done.");
}


void Ds2SampleModule::terminate()
{
  delete m_streamer;

  // RingBuffer should not be deleted

  B2INFO("Ds2Sample: terminate called")
}

