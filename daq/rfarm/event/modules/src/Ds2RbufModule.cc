//+
// File : Ds2RbufModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/Ds2RbufModule.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Ds2Rbuf)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Ds2RbufModule::Ds2RbufModule() : Module()
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
  B2INFO("Ds2Rbuf: Constructor done.");
}


Ds2RbufModule::~Ds2RbufModule()
{
}

void Ds2RbufModule::initialize()
{

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(m_rbufname.c_str());
  //  m_msghandler = new MsgHandler(m_compressionLevel);
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  B2INFO("Ds2Rbuf initialized.");
}


void Ds2RbufModule::beginRun()
{
  B2INFO("Ds2Rbuf: beginRun called.");
}


void Ds2RbufModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);
  //  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event, false, true);

  B2INFO("Ds2Rbuf: msgsize = " << msg->size());

  //  printf("message size = %d\n", msg->size());
  // Put the message in ring buffer
  for (;;) {
    int stat = m_rbuf->insq((int*)msg->buffer(), (msg->size() - 1) / 4 + 1);
    if (stat >= 0) break;
    //    usleep(200);
    usleep(20);
  }

  B2INFO("Ds2Rbuf: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete msg;

  // return
  m_nsent++;

}

void Ds2RbufModule::endRun()
{
  //fill Run data

  printf("Ds2Rbuf: endRun called.....\n");
  B2INFO("Ds2Rbuf: endRun done.");
}


void Ds2RbufModule::terminate()
{
  delete m_streamer;

  // RingBuffer should not be deleted

  B2INFO("Ds2Rbuf: terminate called")
}

