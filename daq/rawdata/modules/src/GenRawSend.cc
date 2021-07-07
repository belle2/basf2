/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rawdata/modules/GenRawSend.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#define HEADER_SIZE 23

using namespace std;
using namespace Belle2;

// Global variable
//RawSend* RAWCDC = NULL;

// Framework interface
REG_MODULE(GenRawSend)

// Constructor and destructor

GenRawSendModule::GenRawSendModule() : Module()
{
  //Set module properties
  setDescription("GenRawSendModule module");
  //printf("GenRawSendModule: Constructor called \n");

  m_buffer = NULL;
  m_nevt = 0;
  //Parameter definition
  addParam("EventDataBufferWords", BUF_SIZE_WORD, "DataBuffer words per event", 4800);
  addParam("Size", m_size, "Raw Data Size", 256);
  addParam("MaxEvent", m_maxevt, "Max Number of Event", 1000000);
  addParam("DestHostName", m_dest, "Destination host", string("localhost"));
  addParam("DestPort", m_port, "Destination port", 1111);


  //cout << "GenRawSendModule : constructor called" << endl;
}


GenRawSendModule::~GenRawSendModule()
{
  //printf("GenRawSendModule: Destructor called \n");

}

// Module functions

void GenRawSendModule::initialize()
{
  m_buffer = new char[ BUF_SIZE_WORD ];
  memset(m_buffer, 0,  BUF_SIZE_WORD);

  // Initialize EventMetaData
  //  StoreObjPtr<EventMetaData>::registerPersistent();
  //Register the EventMetaData in the data store
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize RawCOPPER
  m_rawcopper.registerInDataStore();

  // Open Socket
  m_sock = new EvtSocketSend(m_dest, m_port);

}

void GenRawSendModule::beginRun()
{
}


void GenRawSendModule::event()
{

  // Max event number
  if (m_nevt > m_maxevt) return;

  // Event Meta Data
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nevt);

  /*
  // Header
  unsigned int hdr[8];
  hdr[0] = m_size + 8;
  hdr[1] = m_nevt;

  RawHeader rhdr ( hdr );
  */

  // Create RawCOPPER and fill buffer
//   m_rawcopper.create();
//   int* buf = m_rawcopper->AllocateBuffer(m_size + HEADER_SIZE);
//   buf[0] = m_size + HEADER_SIZE;
//   buf[1] = HEADER_SIZE;
//   buf[2] = m_size;
//   //  memset ( (char*)&buf[HEADER_SIZE+1], 0, (m_size-1)*4 );
//   memcpy((char*)&buf[HEADER_SIZE + 1], m_buffer, (m_size - 1) * 4);

//   /* Dummy Data
//   int* buf = (int*)m_buffer;
//   buf[0] = m_size+HEADER_SIZE;
//   */

//   // Send Buffer
//   int nbytes = buf[0] * 4;
//   int stat = m_sock->send_buffer(nbytes, (char*)buf);
//   //  printf ( "nbytes = %d, stat = %d\n", nbytes, stat );

  m_nevt++;
}


void GenRawSendModule::endRun()
{

}


void GenRawSendModule::terminate()
{
  printf("GenRawSendModule : termination called\n");

}

