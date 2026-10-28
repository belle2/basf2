/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
  while (!m_rbuf->isDead()) {
    const int size = m_rbuf->remq(m_evtbuf.data(), m_evtbuf.size());
    if (size > 0) {
      B2DEBUG(35, "Rx: got an event from RingBuffer, size=" << size);

      // Restore objects in DataStore
      EvtMessage evtmsg(reinterpret_cast<char*>(m_evtbuf.data()));
      m_streamer->restoreDataStore(&evtmsg);
      // Restore the event dependent random number object from Datastore
      if (m_randomgenerator.isValid()) {
        RandomNumbers::getEventRandomGenerator() = *m_randomgenerator;
      }
      break;
    }
    if (size < 0) {
      // The record was too big for m_evtbuf, so remq() left it queued and returned minus the
      // size it needs: grow the buffer and loop round to read the very same record. The 50%
      // headroom keeps the next slightly larger event from triggering another round trip.
      m_evtbuf.resize(-size + -size / 2);
      B2DEBUG(32, "Rx: grew receive buffer to " << m_evtbuf.size() << " words");
    } else {
      usleep(20);
    }
  }
}

void RxModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Start small and non-empty; readEvent() grows it to whatever the records need.
  // 4096 words -> 16 kB
  m_evtbuf.resize(4096);

  m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);

  initStreamer();

  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).
  readEvent();
}

void RxModule::event()
{
  m_nrecv++;
  // First event is already loaded in initialize()
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  readEvent();
}

void RxModule::terminate()
{
  B2DEBUG(32, "Rx: terminate called");
  delete m_streamer;
}
