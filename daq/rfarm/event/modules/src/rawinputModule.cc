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
  StoreArray<RawDataBlock>::registerPersistent();
  StoreArray<RawCOPPER>::registerPersistent();
  StoreArray<RawSVD>::registerPersistent();
  StoreArray<RawCDC>::registerPersistent();
  StoreArray<RawTOP>::registerPersistent();
  StoreArray<RawARICH>::registerPersistent();
  StoreArray<RawECL>::registerPersistent();
  StoreArray<RawKLM>::registerPersistent();
  StoreArray<RawTRG>::registerPersistent();
  StoreArray<RawFTSW>::registerPersistent();

  // Open input file
  printf("RawInput : Opening file %s\n", m_inputFileName.c_str());
  m_fd = open(m_inputFileName.c_str(), O_RDONLY);
  if (m_fd <= 0) {
    perror("fopen");
    exit(-1);
  }
  printf("Done. m_fd = %d\n", m_fd);

  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with SimpleOutput.
  //  ---- Prefetch the first event
  registerRawCOPPERs();

  B2INFO("RawInput: initialized.");
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

void RawInputModule::registerRawCOPPERs()
{

  // Get a record from a file
  int error_flag = 0;
  char* evtbuf = new char[MAXEVTSIZE];
  // Get a record from file
  int sstat = read(m_fd, evtbuf, sizeof(int));
  if (sstat <= 0) {
    delete[] evtbuf;
    return;
  }
  int* recsize = (int*)evtbuf;
  int rstat = read(m_fd, evtbuf + sizeof(int), (*recsize - 1) * 4);
  if (rstat <= 0) {
    delete[] evtbuf;
    return;
  }

  B2INFO("RawInput: got an event from a file, size=" << recsize <<
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
  B2INFO("RawInput: Ncprs=" << ncprs << " Nwords=" << nwords);

  // Get buffer header
  int* bufbody = (int*)evtbuf + SendHeader::SENDHDR_NWORDS;

  //
  // Copy from Raw2DsModule.cc -- From here
  //

  // Unpack buffer
  RawDataBlock tempdblk;
  tempdblk.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);

  // Store data contents in Corresponding RawXXXX
  for (int cprid = 0; cprid < ncprs * npackedevts; cprid++) {
    // Pick up one COPPER and copy data in a temporary buffer
    int nwds_buf = tempdblk.GetBlockNwords(cprid);
    int* cprbuf = new int[nwds_buf];
    memcpy(cprbuf, tempdblk.GetBuffer(cprid), nwds_buf * 4);

    // Check FTSW
    if (tempdblk.CheckFTSWID(cprid)) {
      StoreArray<RawFTSW> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      continue;
    }

    RawCOPPER tempcpr;
    //    tempcpr.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);  -> bug. If RawFTSW is stored in bufbody, tempcpr's version becomes 0 and getNodeID fails.
    tempcpr.SetBuffer(cprbuf, nwds_buf, false, 1, 1);

    //    int subsysid = ((RawCOPPER&)tempdblk).GetNodeID(cprid);
    //    int subsysid = tempcpr.GetNodeID(cprid);
    int subsysid = tempcpr.GetNodeID(0);
    if (tempcpr.GetEventCRCError(0) != 0) error_flag = 1;

    // Switch to each detector and register RawXXX
    if ((subsysid & DETECTOR_MASK) == CDC_ID) {
      StoreArray<RawCDC> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == SVD_ID) {
      StoreArray<RawSVD> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == BECL_ID) {
      StoreArray<RawECL> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == EECL_ID) {
      StoreArray<RawECL> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == TOP_ID) {
      StoreArray<RawTOP> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == ARICH_ID) {
      StoreArray<RawARICH> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == BKLM_ID) {
      StoreArray<RawKLM> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == EKLM_ID) {
      StoreArray<RawKLM> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if (((subsysid & DETECTOR_MASK) & 0xF0000000) == TRGDATA_ID) {
      StoreArray<RawTRG> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else {
      StoreArray<RawCOPPER> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    }
    //    delete[] cprbuf;
  }

  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();
  evtmetadata->setExperiment(sndhdr.GetExpNum());
  evtmetadata->setRun(sndhdr.GetRunNum());
  evtmetadata->setSubrun(sndhdr.GetSubRunNum());
  evtmetadata->setEvent(sndhdr.GetEventNumber());
  if (error_flag) evtmetadata->addErrorFlag(EventMetaData::c_B2LinkEventCRCError);

  delete[] evtbuf;
  // Copy from Raw2DsModule.cc -- Up to here

  B2INFO("RawInput: DataStore Restored!!");
  return;
}


void RawInputModule::endRun()
{
  B2INFO("RawInput: endRun done.");
}


void RawInputModule::terminate()
{
  close(m_fd);
  B2INFO("RawInput: terminate called");
}

