//+
// File : DeSerializerPXD.cc
// Description : Module to receive PXD Data from external socket and store it as RawPXD in Data Store
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Aug - 2013
//-

#include <daq/rawdata/modules/DeSerializerPXD.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerPXD)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerPXDModule::DeSerializerPXDModule() : Module()
{
  //Set module properties
  setDescription("Receives PXD-Data from ONSEN (or a simulator) and stores it as RawPXD in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  addParam("Port", m_port, "default port number", 11112);
  addParam("HostName", m_host, "default host name");
  m_nsent = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];
  events_processed = 0;
  //Parameter definition
  B2DEBUG(0, "DeSerializerPXDModule: Constructor done.");
}


DeSerializerPXDModule::~DeSerializerPXDModule()
{
  delete[] m_buffer;
}

void DeSerializerPXDModule::initialize()
{
  // Open receiver sockets
  m_recv = new EvtSocketSend(m_host, m_port);

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerAsPersistent();

  // Initialize Array of RawCOPPER
  StoreArray<RawPXD>::registerPersistent();

  B2DEBUG(0, "DeSerializerPXDModule: initialized.");
}

void DeSerializerPXDModule::beginRun()
{
  B2DEBUG(0, "beginRun called.");
}

void DeSerializerPXDModule::event()
{
  // DataStore interface
  StoreArray<RawPXD> rawpxdary;

  // Get a record from socket
  int stat;
  do {
    stat = m_recv->recv_pxd_buffer((char*)m_buffer);
//        if(stat==0) {
//            sleep(1);
//        };
    if (stat <= 0) {
      //          B2INFO("stat : " << stat);
      DeSerializerPXDModule::endRun();
      DeSerializerPXDModule::terminate();
      return;
    };
  } while (stat == 0);
  /*
   B2INFO("DeSer: port = " << m_port << " stat = " << stat << " : size=" << m_buffer[1]
          << " hdr = " << std::hex << m_buffer[0] << std::dec <<  " dummy1 = " << m_buffer[2]
          <<  " dummy2 = " << m_buffer[3]);
   */
//    int i=0;
//    do {
//        i++;
//    }
//    while(m_buffer[4+i]!=0);
  /*
     B2INFO(" Nr: " << events_processed++ << " Magic: " << m_buffer[0] << " length of data " << m_buffer[1]);
    for (int j=i-1;j<(m_buffer[1]+36)/4;j++)
     {
       //B2INFO(j-i+2 << " DATA: " << htonl(m_buffer[j+6]));
     }
  */
  // Fill RawPXD
  RawPXD rawpxd(m_buffer, stat); //stat=lenght_in_Bytes

  // Put it in DataStore
  rawpxdary.appendNew(rawpxd);


  // Update EventMetaData
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nsent);

  m_nsent++;

  return;
}

void DeSerializerPXDModule::endRun()
{
  //fill Run data

  B2DEBUG(0, "endRun done.");
}


void DeSerializerPXDModule::terminate()
{
  B2INFO("terminate called");
}

