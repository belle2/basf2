/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/storage/modules/Ds2RawFileModule.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <fcntl.h>

using namespace std;
using namespace Belle2;

//!! for debugging only
// static int fdout;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Ds2RawFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Ds2RawFileModule::Ds2RawFileModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  //addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
  //         string("OutputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);

  //m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Ds2Raw: Constructor done.");
}


Ds2RawFileModule::~Ds2RawFileModule()
{
}

void Ds2RawFileModule::initialize()
{

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  //m_rbuf = new RingBuffer(m_rbufname.c_str());
  m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_file = open("file.data", O_CREAT | O_RDWR | O_TRUNC, 0644);

  //!! For debugging only
  //!!  fdout = open ( "Ds2RawDump3.dat", O_CREAT|O_RDWR|O_TRUNC, 0666 );

  B2INFO("Ds2Raw initialized.");
}


void Ds2RawFileModule::beginRun()
{
  B2INFO("Ds2Raw: beginRun called.");
}


void Ds2RawFileModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(DataStore::c_Event);

  // Event Meta Data
  StoreObjPtr<EventMetaData> evtmeta;

  // Fill Header and Trailer
  SendHeader hdr;
  SendTrailer trl;

  // Number of total words
  int msgsize = (msg->size() - 1) / 4 + 1;
  int total_nwrds = msgsize +  hdr.GetHdrNwords() + trl.GetTrlNwords();

  // Fill header and trailer
  hdr.SetNwords(total_nwrds);
  hdr.SetNumEventsinPacket(1);
  hdr.SetNumNodesinPacket(1);
  hdr.SetEventNumber(evtmeta->getEvent());
  //  hdr.SetExpRunWord(evtmeta->getRun());
  hdr.SetSubRunNum(0);     // temporary
  hdr.SetRunNum(evtmeta->getRun());
  hdr.SetExpNum(evtmeta->getExperiment());
  hdr.SetNodeID(HLT_SUBSYS_ID);
  //  trl.SetMagicWord();

  // Allocate raw buffer
  int* buffer = new int[total_nwrds];

  // Fill header
  memcpy(buffer, hdr.GetBuffer(), hdr.GetHdrNwords()*sizeof(int));

  // Fill EvtMessage
  memcpy(buffer + hdr.GetHdrNwords(), msg->buffer(), msg->size());

  // Fill trailer
  memcpy(buffer + hdr.GetHdrNwords() + msgsize, trl.GetBuffer(),
         trl.GetTrlNwords()*sizeof(int));

  write(m_file, buffer, total_nwrds * 4);

  B2INFO("Ds2Raw: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete[] buffer;
  delete msg;

  // return
  m_nsent++;

}

void Ds2RawFileModule::endRun()
{
  //fill Run data

  printf("Ds2Raw: endRun called.....\n");
  B2INFO("Ds2Raw: endRun done.");
}


void Ds2RawFileModule::terminate()
{
  delete m_streamer;

  // RingBuffer should not be deleted

  B2INFO("Ds2Raw: terminate called");
}

