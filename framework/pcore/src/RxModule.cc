/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RxModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <framework/pcore/RxModule.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/core/RandomNumbers.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

RxModule::RxModule(RingBuffer* rbuf) : Module(), m_streamer(nullptr), m_nrecv(-1)
{
  //Set module properties
  setDescription("Decode data from RingBuffer into DataStore");
  setPropertyFlags(c_Input | c_InternalSerializer);
  setType("Rx");

  m_rbuf = rbuf;
  m_compressionLevel = 0;
  if (rbuf) {
    setName("Rx" + std::to_string(rbuf->shmid()));
    B2DEBUG(32, "Rx: Constructor with RingBuffer done.");
  }
}



RxModule::~RxModule() = default;

void RxModule::initStreamer()
{
  delete m_streamer;
  m_streamer = new DataStoreStreamer(m_compressionLevel, m_handleMergeable);
}

void RxModule::readEvent()
{
  auto* evtbuf = new char[EvtMessage::c_MaxEventSize];
  while (!m_rbuf->isDead()) {
    int size = m_rbuf->remq((int*)evtbuf);
    if (size != 0) {
      B2DEBUG(35, "Rx: got an event from RingBuffer, size=" << size);

      // Restore objects in DataStore
      EvtMessage evtmsg(evtbuf);
      m_streamer->restoreDataStore(&evtmsg);
      // Restore the event dependent random number object from Datastore
      if (m_randomgenerator.isValid()) {
        RandomNumbers::getEventRandomGenerator() = *m_randomgenerator;
      }
      break;
    }
    usleep(20);
  }

  delete[] evtbuf;
}

void RxModule::initialize()
{
  gSystem->Load("libdataobjects");

  m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);

  initStreamer();

  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).
  readEvent();
}

void RxModule::beginRun() { }

void RxModule::event()
{
  m_nrecv++;
  // First event is already loaded in initialize()
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  readEvent();
}

void RxModule::endRun() { }

void RxModule::terminate()
{
  B2DEBUG(32, "Rx: terminate called");
  delete m_streamer;
}
