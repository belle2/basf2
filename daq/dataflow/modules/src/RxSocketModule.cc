/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/dataflow/modules/RxSocketModule.h>

#include <TSystem.h>

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
  m_nrecv = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


RxSocketModule::~RxSocketModule()
{
}

void RxSocketModule::initialize()
{
  // Load data objects definitions
  gSystem->Load("libdataobjects");

  // Open receiving socekt
  m_recv = new EvtSocketRecv(m_port);

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  // Prefetch first record in RxSocket
  EvtMessage* msg = m_recv->recv();
  if (msg == NULL) {
    return;
  }
  m_streamer->restoreDataStore(msg);

  // Delete buffers
  delete msg;

  m_nrecv = -1;

  B2INFO("Rx initialized.");
}


void RxSocketModule::beginRun()
{
  B2INFO("beginRun called.");
}


void RxSocketModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;

  // Get a record from socket
  EvtMessage* msg = m_recv->recv();
  if (msg == NULL) {
    return;
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

void RxSocketModule::endRun()
{
  //fill Run data

  B2INFO("endRun done.");
}


void RxSocketModule::terminate()
{
  B2INFO("terminate called");
}

