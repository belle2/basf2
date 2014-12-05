//+
// File : PXDReadRawONSEN.cc
// Description : Module to Load Raw PXD Data from ONSEN network-dump file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 15 - Apr - 2014
//-

#include <pxd/modules/pxdUnpacking/PXDReadRawONSEN.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;
using namespace PXD;

using namespace boost::spirit::endian;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDReadRawONSEN)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDReadRawONSENModule::PXDReadRawONSENModule() : Module()
{
  fh = 0;
  m_msghandler = 0;
  //Set module properties
  setDescription("Read a Raw PXD-Data Dump from ONSEN (or a simulator) and stores it as RawPXD in Data Store");
  //setPropertyFlags(c_Input | c_ParallelProcessingCertified);
  //setPropertyFlags(c_Input);

  addParam("FileName", m_filename, "file name");
  m_nread = 0;
  m_compressionLevel = 0;
  m_buffer = new int[MAXEVTSIZE];

  B2DEBUG(0, "PXDReadRawONSENModule: Constructor done.");
}


PXDReadRawONSENModule::~PXDReadRawONSENModule()
{
  delete[] m_buffer;
}

void PXDReadRawONSENModule::initialize()
{
  // Open receiver sockets
//  m_recv = new EvtSocketSend(m_host, m_port);
  fh = fopen(m_filename.c_str(), "rb");
  if (fh) {
    B2INFO("Read Raw ONSEN Data from " << m_filename);
  } else {
    B2ERROR("Could not open Raw ONSEN Data: " << m_filename);
  }

  // Open message handler
  m_msghandler = new MsgHandler(m_compressionLevel);

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize Array of RawCOPPER
  StoreArray<RawPXD> storeRawPIDs;
  storeRawPIDs.registerInDataStore();

  B2DEBUG(0, "PXDReadRawONSENModule: initialized.");
}

void PXDReadRawONSENModule::beginRun()
{
  B2DEBUG(0, "beginRun called.");
}

int PXDReadRawONSENModule::read_data(char* data, size_t len)
{
  size_t l = 0;
  if (fh) l = fread(data, 1, len, fh);
  if (l != len) return 0;
  return l;
}

int PXDReadRawONSENModule::readOneEvent()
{
  char* data = (char*)m_buffer;
  int len = MAXEVTSIZE * sizeof(int);

#define MAX_PXD_FRAMES  256
  const int headerlen = 8;
  ubig32_t* pxdheader = (ubig32_t*) data;
  ubig32_t* pxdheadertable = (ubig32_t*) &data[headerlen];
  int framenr = 0, tablelen = 0, datalen = 0;
  int br = read_data(data, headerlen);
  if (br <= 0) return br;
  if (pxdheader[0] != (int)0xCAFEBABE) {
    printf("pxdheader wrong : Magic %X , Frames %X \n", (unsigned int) pxdheader[0], (unsigned int) pxdheader[1]);
    exit(0);
  }
  framenr = pxdheader[1];
  if (framenr > MAX_PXD_FRAMES) {
    printf("MAX_PXD_FRAMES too small : %d(%d) \n", framenr, MAX_PXD_FRAMES);
    exit(0);
  }
  tablelen = 4 * framenr;
  br = read_data((char*)&data[headerlen], tablelen);
  if (br <= 0) return br;
  for (int i = 0; i < framenr; i++) {
    datalen += (pxdheadertable[i] + 3) & 0xFFFFFFFC;
  }

  if (datalen + headerlen + tablelen > len) {
    printf("buffer too small : %d %d %d(%d) \n", headerlen, tablelen, datalen, len);
    exit(0);
  }
  int bcount = read_data(data + headerlen + tablelen, datalen);
  if (br <= 0) return br;
  return (headerlen + tablelen + bcount);
}

void PXDReadRawONSENModule::event()
{
  if (fh == 0) {
    B2ERROR("Unexpected close of dump file.");
    PXDReadRawONSENModule::endRun();
    PXDReadRawONSENModule::terminate();
    return;
  }
  // DataStore interface
  StoreArray<RawPXD> rawpxdary;

  // Get a record from socket
  int stat;
  do {
    stat = readOneEvent();
    if (stat <= 0) {
      /// End of File
      PXDReadRawONSENModule::endRun();
      PXDReadRawONSENModule::terminate();
      return;
    };
  } while (stat == 0);

  // Fill RawPXD
  RawPXD rawpxd(m_buffer, stat); //stat=lenght_in_Bytes

  // Put it in DataStore
  rawpxdary.appendNew(rawpxd);


  // Update EventMetaData
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(m_nread);

  m_nread++;

  return;
}

void PXDReadRawONSENModule::endRun()
{
  //fill Run data

  B2DEBUG(0, "endRun done.");
}


void PXDReadRawONSENModule::terminate()
{
  B2INFO("terminate called");
  if (fh) fclose(fh);
  fh = 0;
}

