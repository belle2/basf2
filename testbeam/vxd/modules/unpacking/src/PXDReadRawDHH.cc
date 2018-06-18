//+
// File : PXDReadRawDHH.cc
// Description : Module to Load Raw PXD Data from DHH network-dump file and store it as RawDHH in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 07.04.2916
//-

#include <testbeam/vxd/modules/unpacking/PXDReadRawDHH.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDReadRawDHH)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDReadRawDHHModule::PXDReadRawDHHModule() : Module()
{
  fh = 0;
  m_msghandler = 0;
  //Set module properties
  setDescription("Read a Raw PXD-Data Dump from DHH (or a simulator) and stores it as RawDHH in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  //setPropertyFlags(c_Input);

  addParam("FileName", m_filename, "file name");
  m_nread = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];

  B2DEBUG(0, "PXDReadRawDHHModule: Constructor done.");
}


PXDReadRawDHHModule::~PXDReadRawDHHModule()
{
  delete[] m_buffer;
}

void PXDReadRawDHHModule::initialize()
{
  // Open receiver sockets
//  m_recv = new EvtSocketSend(m_host, m_port);
  fh = fopen(m_filename.c_str(), "rb");
  if (fh) {
    B2INFO("Read Raw DHH Data from " << m_filename);
  } else {
    B2ERROR("Could not open Raw DHH Data: " << m_filename);
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize Array of RawCOPPER
  StoreArray<RawDHH> storeRawPIDs;
  storeRawPIDs.registerInDataStore();

  B2DEBUG(0, "PXDReadRawDHHModule: initialized.");
}

void PXDReadRawDHHModule::beginRun()
{
  B2DEBUG(0, "beginRun called.");
}

int PXDReadRawDHHModule::read_data(char* data, size_t len)
{
  size_t l = 0;
  if (fh) l = fread(data, 1, len, fh);
  if (l != len) return 0;
  return l;
}

int PXDReadRawDHHModule::readOneEvent()
{

  struct EvtHeader {
    unsigned int    EventSize: 20;
    unsigned short   flag0: 1;
    unsigned short   flag1: 1;
    unsigned short  EventType: 2;
    unsigned short  ModuleNo: 4;
    unsigned short  DeviceType: 4;
    unsigned int    Triggernumber;
  };

  char* data = (char*)m_buffer;
  struct EvtHeader* evt1 = (struct EvtHeader*)data;
  struct EvtHeader* evt2 = (struct EvtHeader*)(data + 8);

  unsigned int len = 0;
  while (1) {
    // Read 8 bytes header (group)
    int br = read_data(data, 8);
    if (br <= 0) return br;

    len = (evt1->EventSize) * 4;
    B2INFO("DHH Reader: TrgNr1: " << evt1->Triggernumber << " Len " << len << " Type " << evt1->EventType);

    len -= 8;
    if (len >= 8) {
      br = read_data(data + 8, 8);
      if (br <= 0) return br;
      // Read 8 bytes header (client)
      // compare len
      unsigned int data_size = (evt2->EventSize) * 4;
      if (data_size != len) {
        B2ERROR("DHH Reader: Lengh of header1-8 and header2 do not match" << len << "!=" << data_size);
      }
      B2INFO("DHH Reader: TrgNr2: " << evt2->Triggernumber << " Len " << len << " Type " << evt2->EventType);
      len -= 8;
      if (len > 0) {
        B2INFO("Read Event len " << len << "+16");
        m_nread = evt2->Triggernumber;
        // now comes the best part ... we found a valid packet and end loop
        break;
      }
    }
  }

  int br = read_data(data + 16, len);
  if (br <= 0) return br;

  return (len + 16);
}

void PXDReadRawDHHModule::event()
{
  if (fh == 0) {
    B2ERROR("Unexpected close of dump file.");
    PXDReadRawDHHModule::endRun();
    PXDReadRawDHHModule::terminate();
    return;
  }
  // DataStore interface
  StoreArray<RawDHH> rawdhhary;

  // Get a record from socket
  int stat;
  do {
    stat = readOneEvent();
    if (stat <= 0) {
      /// End of File
      PXDReadRawDHHModule::endRun();
      PXDReadRawDHHModule::terminate();
      return;
    };
  } while (stat == 0);

  // Fill RawDHH in DataStore, stat=lenght_in_Bytes
  rawdhhary.appendNew(m_buffer, stat);


  // Update EventMetaData
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nread);

//   m_nread++;

  return;
}

void PXDReadRawDHHModule::endRun()
{
  //fill Run data

  B2DEBUG(0, "endRun done.");
}


void PXDReadRawDHHModule::terminate()
{
  B2INFO("terminate called");
  if (fh) fclose(fh);
  fh = 0;
}

