//+
// File : Raw2DsModule.cc
// Description : Module to restore DataStore from RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <daq/rfarm/event/modules/Raw2DsModule.h>
#include <TSystem.h>
#include <stdlib.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

// #define DESY

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Raw2Ds)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Raw2DsModule::Raw2DsModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_Input);

  addParam("RingBufferName", m_rbufname, "Name of RingBuffer",
           string("InputRbuf"));

  m_rbuf = NULL;
  m_nevt = -1;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


Raw2DsModule::~Raw2DsModule()
{
}

void Raw2DsModule::initialize()
{
  gSystem->Load("libdataobjects");

  m_rbuf = new RingBuffer(m_rbufname.c_str());

  // Initialize EvtMetaData
  StoreObjPtr<EventMetaData>::registerPersistent();

  // Initialize Array of RawCOPPER
  StoreArray<RawCOPPER>::registerPersistent();
  StoreArray<RawSVD>::registerPersistent();
  StoreArray<RawCDC>::registerPersistent();
  StoreArray<RawBPID>::registerPersistent();
  StoreArray<RawEPID>::registerPersistent();
  StoreArray<RawECL>::registerPersistent();
  StoreArray<RawKLM>::registerPersistent();
  StoreArray<RawFTSW>::registerPersistent();

#ifdef DESY
  StoreArray<RawTLU>::registerPersistent();
#endif
  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with SimpleOutput.
  //  ---- Prefetch the first event
  registerRawCOPPERs();

  B2INFO("Rx initialized.");
}


void Raw2DsModule::beginRun()
{
  B2INFO("beginRun called.");
}


void Raw2DsModule::event()
{
  m_nevt++;
  // Skip first event since it is read in initialize();
  if (m_nevt == 0) return;

  registerRawCOPPERs();
}

void Raw2DsModule::registerRawCOPPERs()
{
  // Get a record from ringbuf
  int size;

  int* evtbuf = new int[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    usleep(100);
    //    usleep(20);
  }

  B2INFO("Raw2Ds: got an event from RingBuffer, size=" << size <<
         " (proc= " << (int)getpid() << ")");

  // Unpack SendHeader
  SendHeader sndhdr;
  sndhdr.SetBuffer(evtbuf);
  int npackedevts = sndhdr.GetNumEventsinPacket();
  if (npackedevts != 1) {
    B2FATAL("Raw2DsModule::number of events in packet is not 1");
  }
  int ncprs = sndhdr.GetNumNodesinPacket();
  int nwords = sndhdr.GetTotalNwords() - SendHeader::SENDHDR_NWORDS - SendTrailer::SENDTRL_NWORDS;

  B2INFO("Raw2DS: Ncprs=" << ncprs << " Nwords=" << nwords);
  // Get buffer header
  int* bufbody = evtbuf + SendHeader::SENDHDR_NWORDS;

  // Unpack buffer
  RawDataBlock tempdblk;
  //  RawCOPPER tempdblk;
  tempdblk.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);

  // Store data contents in Corresponding RawXXXX
  for (int cprid = 0; cprid < ncprs; cprid++) {
    // Pick up one COPPER and copy data in a temporary buffer
    int nwds_buf = tempdblk.GetBlockNwords(cprid);
    int* cprbuf = new int[nwds_buf];
    memcpy(cprbuf, tempdblk.GetBuffer(cprid), nwds_buf * 4);
    // Get subsys id
    // Check FTSW
    if (tempdblk.CheckFTSWID(cprid)) {
      StoreArray<RawFTSW> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      continue;
    }
#ifdef DESY
    if (tempdblk.CheckTLUID(cprid)) {
      StoreArray<RawTLU> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      continue;
    }
#endif

    int subsysid = ((RawCOPPER&)tempdblk).GetSubsysId(cprid);
    //    subsysid = (subsysid & 0xff000000) >> 24;
    //    printf("#################%.8x\n", subsysid);

    // Switch to each detector and register RawXXX
    if ((subsysid & DETECTOR_MASK) == CDC_ID) {
      StoreArray<RawCDC> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == SVD_ID) {
      StoreArray<RawSVD> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == ECL_ID) {
      StoreArray<RawECL> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == BPID_ID) {
      StoreArray<RawBPID> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == EPID_ID) {
      StoreArray<RawEPID> ary;
      (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    } else if ((subsysid & DETECTOR_MASK) == KLM_ID) {
      StoreArray<RawKLM> ary;
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
  evtmetadata->setEvent(sndhdr.GetEventNumber());
  //  printf ( "ExpNo = %d, RunNo = %d, EvtNo = %d\n", sndhdr.GetExpNum(),
  //       sndhdr.GetRunNum(), sndhdr.GetEventNumber() );
  delete[] evtbuf;

  B2INFO("Raw2Ds: DataStore Restored!!");
  return;
}

void Raw2DsModule::endRun()
{
  //fill Run data

  B2INFO("Raw2Ds: endRun done.");
}


void Raw2DsModule::terminate()
{
  B2INFO("Raw2Ds: terminate called")
}

