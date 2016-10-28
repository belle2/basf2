//+
// File : Ds2RawModule.cc
// Description : Module to encode DataStore and place it in Ringbuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/Ds2RawModule.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace Belle2;

//!! for debugging only
// static int fdout;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Ds2Raw)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Ds2RawModule::Ds2RawModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("OutputRbuf"));
  addParam("CompressionLevel", m_compressionLevel, "Compression level",
           0);

  m_rbuf = NULL;
  m_nsent = 0;
  m_compressionLevel = 0;

  //Parameter definition
  B2INFO("Ds2Raw: Constructor done.");
}


Ds2RawModule::~Ds2RawModule()
{
}

void Ds2RawModule::initialize()
{

  //  m_rbuf = new RingBuffer(m_rbufname.c_str(), RBUFSIZE);
  m_rbuf = new RingBuffer(m_rbufname.c_str());
  m_streamer = new DataStoreStreamer(m_compressionLevel);

  //!! For debugging only
  //!!  fdout = open ( "Ds2RawDump3.dat", O_CREAT|O_RDWR|O_TRUNC, 0666 );

  B2INFO("Ds2Raw initialized.");
}


void Ds2RawModule::beginRun()
{
  B2INFO("Ds2Raw: beginRun called.");
}


void Ds2RawModule::event()
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
  hdr.SetSubRunNum(evtmeta->getSubrun());     // modified on Apr. 20, 2016 by SY
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
  //  printf ( "trailer = %8.8x, %8.8x\n", *(buffer+hdr.GetHdrNwords()+msgsize),
  //       *(buffer+hdr.GetHdrNwords()+msgsize+1) );

  // Put the raw buffer in ring buffer
  for (;;) {
    int stat = m_rbuf->insq(buffer, total_nwrds);
    if (stat >= 0) break;
    //    usleep(200);
    usleep(20);
  }

  //!! For debugging only
  //!! int is = write ( fdout, buffer, total_nwrds*4 );

  B2INFO("Ds2Raw: objs sent in buffer. Size = " << msg->size());

  // Release EvtMessage buffer
  delete[] buffer;
  delete msg;

  // return
  m_nsent++;

}

void Ds2RawModule::endRun()
{
  //fill Run data

  printf("Ds2Raw: endRun called.....\n");
  B2INFO("Ds2Raw: endRun done.");
}


void Ds2RawModule::terminate()
{
  delete m_streamer;

  // RingBuffer should not be deleted

  B2INFO("Ds2Raw: terminate called");
}

