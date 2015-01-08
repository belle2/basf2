//+
// File : rawinput.cc
// Description : Sequential ROOT input module for pbasf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012     modified to use DataStoreStreamer, clean up
//-

#include <daq/rfarm/event/modules/rawinputModule.h>

#include "TSystem.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RawInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RawInputModule::RawInputModule() : Module()
{
  //Set module properties
  setDescription("Raw data file input module");
  setPropertyFlags(c_Input);

  m_fd = 0;
  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "Raw file name.", string("RawInput.root"));

  B2DEBUG(1, "RawInput: Constructor done.");
}


RawInputModule::~RawInputModule()
{
}

void RawInputModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Initialize EvtMetaData
  StoreObjPtr<EventMetaData>::registerPersistent();

  // Initialize Array of RawCOPPER
  StoreArray<RawCOPPER>::registerPersistent();
  StoreArray<RawSVD>::registerPersistent();
  StoreArray<RawCDC>::registerPersistent();
  StoreArray<RawTOP>::registerPersistent();
  StoreArray<RawARICH>::registerPersistent();
  StoreArray<RawECL>::registerPersistent();
  StoreArray<RawKLM>::registerPersistent();
  StoreArray<RawFTSW>::registerPersistent();

  // Open input file
  printf("RawInput : Opening file %s\n", m_inputFileName.c_str());
  m_fd = open(m_inputFileName.c_str(), O_RDONLY);
  if (m_fd <= 0) {
    perror("fopen");
    exit(-1);
  }
  printf("Done. m_fd = %d\n", m_fd);

  //  ---- Prefetch the first event
  registerRawCOPPERs();

  B2INFO("RawInput: initialized.");
}

void RawInputModule::registerRawCOPPERs()
{
  // BUffer
  char* evtbuf = new char[MAXEVTSIZE];
  // Get a record from file
  int sstat = read(m_fd, evtbuf, sizeof(int));
  if (sstat <= 0) return;
  int* recsize = (int*)evtbuf;
  int rstat = read(m_fd, evtbuf + sizeof(int), (*recsize - 1) * 4);
  if (rstat <= 0) return;

  B2INFO("RawInput: got an event from RingBuffer, size=" << recsize <<
         " (proc= " << (int)getpid() << ")");

  // Unpack SendHeader
  SendHeader sndhdr;
  sndhdr.SetBuffer((int*)evtbuf);
  int npackedevts = sndhdr.GetNumEventsinPacket();
  if (npackedevts != 1) {
    B2FATAL("Raw2DsModule::number of events in packet is not 1");
  }
  int ncprs = sndhdr.GetNumNodesinPacket();
  int nwords = sndhdr.GetTotalNwords() - SendHeader::SENDHDR_NWORDS - SendTrailer::SENDTRL_NWORDS;

  // Get buffer header
  int* bufbody = (int*)evtbuf + SendHeader::SENDHDR_NWORDS;

  // Unpack buffer
  RawCOPPER tempcpr;
  tempcpr.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);

  //  B2INFO ( "NCoppers = " << ncprs << " Nwords = " << nwords );
  // Store data contents in Corresponding RawXXXX
  for (int cprid = 0; cprid < ncprs; cprid++) {
    // Pick up one COPPER and copy data in a temporary buffer
    int nwds_buf = tempcpr.GetBlockNwords(cprid);
    int* cprbuf = new int[nwds_buf];
    memcpy(cprbuf, tempcpr.GetBuffer(cprid), nwds_buf * 4);

    //    printf ( "Processing COPPER %d: blocknw = %d\n", cprid, nwds_buf );

    if (tempcpr.CheckFTSWID(cprid)) {
      //      printf ( "FTSW detected\n" );
      StoreArray<RawFTSW> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      continue;
    }

    // Get subsys id
    int subsysid = tempcpr.GetSubsysId(cprid) & DETECTOR_MASK;
    //    B2INFO ( "--> subsys ID = " << subsysid << " Block words = " << nwds_buf );
    // Switch to each detector and register RawXXX
    if (subsysid == CDC_ID) {
      //      printf ( "CDC identified\n" );
      StoreArray<RawCDC> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (subsysid == SVD_ID) {
      //      printf ( "SVD identified\n" );
      StoreArray<RawSVD> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (subsysid == BECL_ID  || subsysid == EECL_ID) {
      //      printf ( "ECL identified\n" );
      StoreArray<RawECL> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (subsysid == TOP_ID) {
      //      printf ( "TOP identified\n" );
      StoreArray<RawTOP> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (subsysid == ARICH_ID) {
      //      printf ( "ARICH identified\n" );
      StoreArray<RawARICH> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (subsysid == BKLM_ID || subsysid == EKLM_ID) {
      //      printf ( "KLM identified\n" );
      StoreArray<RawKLM> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else {
      //      printf ( "Fall back COPPER identified\n" );
      StoreArray<RawCOPPER> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    }
    //    printf ( "COPPER %d completed\n", cprid );
    //    delete[] cprbuf;
  }

  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();
  evtmetadata->setExperiment(1);
  evtmetadata->setRun(1);
  evtmetadata->setEvent(m_nevt);

  delete[] evtbuf;

  B2INFO("RawInput: DataStore Restored!!");
  return;
}


void RawInputModule::beginRun()
{
  m_nevt = 0;
  B2INFO("RawInput: beginRun called.");
}


void RawInputModule::event()
{
  m_nevt++;
  // First event is already loaded
  if (m_nevt == 0) return;

  registerRawCOPPERs();
}

void RawInputModule::endRun()
{
  B2INFO("RawInput: endRun done.");
}


void RawInputModule::terminate()
{
  close(m_fd);
  B2INFO("RawInput: terminate called")
}

