//+
// File : ReceiveEvent.cc
// Description : Module to receive event from expreco
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include <daq/expreco/modules/ReceiveEvent.h>
#include <stdlib.h>

//#define MAXEVTSIZE 400000000

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ReceiveEvent)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ReceiveEventModule::ReceiveEventModule() : Module()
{
  //Set module properties
  setDescription("Receive Event from ExpReco");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Host", m_host, "Receiver host", string("expreco"));
  addParam("Port", m_port, "Receiver Port", 7111);
  m_nrecv = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


ReceiveEventModule::~ReceiveEventModule()
{
}

void ReceiveEventModule::initialize()
{
  // Load data objects definitions
  gSystem->Load("libdataobjects");

  // Open receiving socekt
  m_recv = new EvtSocketSend(m_host.c_str(), m_port);

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  // Prefetch first record in ReceiveEvent
  EvtMessage* msg = m_recv->recv();
  if (msg == NULL) {
    B2FATAL("Did not receive any data, stopping initialization.");
    return;
  }
  m_streamer->restoreDataStore(msg);

  // Delete buffers
  delete msg;

  m_nrecv = -1;

  B2INFO("Rx initialized.");
}


void ReceiveEventModule::beginRun()
{
  B2INFO("beginRun called.");
}


void ReceiveEventModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;

  // Get a record from socket
  EvtMessage* msg = m_recv->recv();
  if (msg == NULL) {
    printf("Connection is closed. Reconnecting.\n");
    int nrepeat = 5000;
    for (;;) {
      int rstat = (m_recv->sock())->reconnect(nrepeat);
      if (rstat == - 1)
        continue;
      else
        break;
    }
    //    return;
  }
  B2INFO("Rx: got an event from Socket, size=" << msg->size());
  // Check for termination record
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Rx: got termination message. Exitting....");
    return;
    // Flag End Of File !!!!!
    //    return msg->type(); // EOF
  }

  // Restore DataStore
  m_streamer->restoreDataStore(msg);
  B2INFO("Rx: DataStore Restored!!");

  // Delete EvtMessage
  delete msg;

  return;
}

void ReceiveEventModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void ReceiveEventModule::terminate()
{
  B2INFO("terminate called");
}

