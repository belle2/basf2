/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/Rbuf2DsModule.h>
#include <TSystem.h>
#include <stdlib.h>
#include <signal.h>

#include "framework/core/Environment.h"

//extern int basf2SignalReceived;


namespace {
// Signal Handler
  static int signalled = 0;
  static void signalHandler(int sig)
  {
    signalled = sig;
    printf("Rbuf2Ds : Signal received\n");
  }
}

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
  m_nrecv = 0;
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

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(m_rbufname.c_str());

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);


  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with SimpleOutput.

  // Prefetch the first record in Ring Buffer
  int size;
  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Rx : evtbuf is not available yet....\n" );
    //    usleep(100);
    if (signalled != 0) break;
    usleep(20);
  }

  // Restore objects in DataStore
  EvtMessage* evtmsg = new EvtMessage(evtbuf);
  m_streamer->restoreDataStore(evtmsg);

  // Delete buffers
  delete evtmsg;
  delete[] evtbuf;

  m_nrecv = -1;

  B2INFO("Rx initialized.");
}


void Rbuf2DsModule::beginRun()
{
  if (Environment::Instance().getNumberProcesses() != 0) {
    struct sigaction s;
    memset(&s, '\0', sizeof(s));
    s.sa_handler = signalHandler;
    sigemptyset(&s.sa_mask);
    if (sigaction(SIGINT, &s, NULL) != 0) {
      B2FATAL("Rbuf2Ds: Error to connect signal handler");
    }
    printf("Ds2Rbuf : Signal Handler installed.\n");
  }
  B2INFO("beginRun called.");
}


void Rbuf2DsModule::event()
{
  m_nrecv++;
  // First event is already loaded
  if (m_nrecv == 0) return;

  // Get a record from ringbuf
  int size;

  char* evtbuf = new char[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    printf ( "Signal Status = %d\n", globalSignalReceived );
    if (signalled != 0) break;
    usleep(100);
    //    usleep(20);
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

  // Restore DataStore
  // Restore DataStore
  m_streamer->restoreDataStore(msg);

  /*
  // Dummy event header for debugging
  // Event Meta Data
  StoreObjPtr<EventMetaData> evtm;
  evtm.create();
  evtm->setExperiment(1);
  evtm->setRun(1);
  evtm->setEvent(m_nrecv);
  */


  delete[] evtbuf;
  delete msg;

  B2INFO("Rbuf2Ds: DataStore Restored!!");
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
  B2INFO("Rbuf2Ds: terminate called");
}

