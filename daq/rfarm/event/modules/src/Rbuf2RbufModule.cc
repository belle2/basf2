/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/Rbuf2RbufModule.h>
#include <TSystem.h>
#include <stdlib.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Rbuf2Rbuf)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Rbuf2RbufModule::Rbuf2RbufModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("InputRbufName", m_name_rbufin, "Name of Input RingBuffer",
           string("RBUFIN"));
  addParam("OutputRbufName", m_name_rbufout, "Name of Output RingBuffer",
           string("RBUFOUT"));

  m_rbufin = NULL;
  m_rbufout = NULL;
  m_nrecv = 0;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


Rbuf2RbufModule::~Rbuf2RbufModule()
{
}

void Rbuf2RbufModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Connect to Ring Buffers
  m_rbufin = new RingBuffer(m_name_rbufin.c_str());
  m_rbufout = new RingBuffer(m_name_rbufout.c_str());

  // Initialize EvtMetaData
  m_eventMetaData.registerInDataStore();

  // Calls event function for TTree
  event();

  // Set m_nrecv to negative to tell the module is initialized
  m_nrecv = -1;

  B2INFO("Rx initialized.");
}


void Rbuf2RbufModule::beginRun()
{
  B2INFO("beginRun called.");
}


void Rbuf2RbufModule::event()
{
  m_nrecv++;
  // First event is already processed
  if (m_nrecv == 0) return;

  // Event buffer
  int size;
  int* evtbuf = new int[MAXEVTSIZE];

  // Get a record from input ringbuf
  while ((size = m_rbufin->remq(evtbuf)) == 0) {
    //    usleep(100);
    usleep(20);
  }

  // Put the record in output ringbuf
  for (;;) {
    int stat = m_rbufout->insq(evtbuf, size);
    if (stat >= 0) break;
    //    usleep(100);
    usleep(20);
  }

  // Set EventMetaData
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();
  evtmetadata->setExperiment(1);
  evtmetadata->setRun(1);
  evtmetadata->setEvent(m_nrecv);

  delete[] evtbuf;

  B2INFO("Rbuf2Rbuf: RingBuffer copied!!");
  return;
  //  return type;
}

void Rbuf2RbufModule::endRun()
{
  //fill Run data

  B2INFO("Rbuf2Rbuf: endRun done.");
}


void Rbuf2RbufModule::terminate()
{
  B2INFO("Rbuf2Rbuf: terminate called");
}

