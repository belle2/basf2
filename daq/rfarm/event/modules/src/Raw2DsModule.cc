/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/Raw2DsModule.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <TSystem.h>
#include <stdlib.h>
#include <signal.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"
#include "framework/core/Environment.h"

// #define DESY

namespace {
// Signal Handler
  static int signalled = 0;
  static void signalHandler(int sig)
  {
    signalled = sig;
    printf("Raw2Ds : Signal received\n");
  }
}

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
  m_eventMetaData.registerInDataStore();

  // Initialize Array of RawCOPPER
  m_rawDataBlock.registerInDataStore();
  m_rawCOPPER.registerInDataStore();
  m_rawSVD.registerInDataStore();
  m_rawCDC.registerInDataStore();
  m_rawTOP.registerInDataStore();
  m_rawARICH.registerInDataStore();
  m_rawECL.registerInDataStore();
  m_rawKLM.registerInDataStore();
  m_rawTRG.registerInDataStore();
  m_rawFTSW.registerInDataStore();

  // Read the first event in RingBuffer and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with SimpleOutput.
  //  ---- Prefetch the first event
  registerRawCOPPERs();

  B2INFO("Rx initialized.");
}


void Raw2DsModule::beginRun()
{
  if (Environment::Instance().getNumberProcesses() != 0) {
    struct sigaction s;
    memset(&s, '\0', sizeof(s));
    s.sa_handler = signalHandler;
    sigemptyset(&s.sa_mask);
    if (sigaction(SIGINT, &s, NULL) != 0) {
      B2FATAL("Rbuf2Ds: Error to connect signal handler");
    }
    printf("Raw2Ds : Signal Handler installed.\n");
  }
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
  unsigned int error_flag = 0;
  int size;
  int* evtbuf = new int[MAXEVTSIZE];
  while ((size = m_rbuf->remq((int*)evtbuf)) == 0) {
    //    usleep(100);
    //    usleep(20);
    if (signalled != 0) return;
    usleep(5);
  }
  //  B2INFO("Raw2Ds: got an event from RingBuffer, size=" << size <<
  //         " (proc= " << (int)getpid() << ")");

  // Unpack SendHeader
  SendHeader sndhdr;
  sndhdr.SetBuffer(evtbuf);
  int npackedevts = sndhdr.GetNumEventsinPacket();
  if (npackedevts != 1) {
    printf("[WARNING] strange SendHeader : ");
    //    for (int i = 0; i < sndhdr.SENDHDR_NWORDS; i++) {
    for (int i = 0; i < 10; i++) {
      printf("0x%.8x ", *(sndhdr.GetBuffer() + i));
    }
    printf("\n"); fflush(stdout);

    B2WARNING("Raw2DsModule::number of events in packet is not 1. This process gets stuck here. Please ABORT the system. (Please see discussion of daqcore channel in https://b2rc.kek.jp/ on 2017. Nov. 30. about why this is not FATAL message.");
    sleep(86400);
  }
  int ncprs = sndhdr.GetNumNodesinPacket();
  int nwords = sndhdr.GetTotalNwords() - SendHeader::SENDHDR_NWORDS - SendTrailer::SENDTRL_NWORDS;
  //  B2INFO("Raw2DS: Ncprs=" << ncprs << " Nwords=" << nwords);


  // Get buffer header
  int* bufbody = evtbuf + SendHeader::SENDHDR_NWORDS;

  // Unpack buffer
  RawDataBlock tempdblk;
  tempdblk.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);

  unsigned int utime = 0;
  unsigned int ctime = 0;
  unsigned long long int mtime = 0;

  int store_time_flag = 0;

  // Store data contents in Corresponding RawXXXX
  for (int cprid = 0; cprid < ncprs * npackedevts; cprid++) {
    // Pick up one COPPER and copy data in a temporary buffer
    int nwds_buf = tempdblk.GetBlockNwords(cprid);
    int* cprbuf = new int[nwds_buf];
    memcpy(cprbuf, tempdblk.GetBuffer(cprid), nwds_buf * 4);

    // Check FTSW
    if (tempdblk.CheckFTSWID(cprid)) {
      StoreArray<RawFTSW> ary;
      RawFTSW* ftsw = ary.appendNew();
      ftsw->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);

      // Tentative for DESY TB 2017
      utime = (unsigned int)(ftsw->GetTTUtime(0));
      ctime = (unsigned int)(ftsw->GetTTCtime(0));
      mtime = 1000000000 * (unsigned long long int)utime + (unsigned long long int)(std::round(ctime / 0.127216));
      store_time_flag = 1;
      continue;
    } else if (store_time_flag == 0) {
      // Tentative until RawFTSW data stream is established. 2018.5.28
      // Not store RawCOPPER here. 2018.11.23
      RawCOPPER tempcpr_time;
      tempcpr_time.SetBuffer(cprbuf, nwds_buf, false, 1, 1);
      utime = (unsigned int)(tempcpr_time.GetTTUtime(0));
      ctime = (unsigned int)(tempcpr_time.GetTTCtime(0));
      mtime = 1000000000 * (unsigned long long int)utime + (unsigned long long int)(std::round(ctime / 0.127216));
      store_time_flag = 1;
    }

    // Set one block to RawCOPPER
    RawCOPPER tempcpr;
    tempcpr.SetBuffer(cprbuf, nwds_buf, false, 1, 1);
    int subsysid = tempcpr.GetNodeID(0);
    //    if (tempcpr.GetEventCRCError(0) != 0) error_flag = 1;
    error_flag |= (unsigned int)(tempcpr.GetDataType(0));

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

      // Do not store Unknown RawCOPPER object. 2018.11.25
      printf("[WARNING] Unknown COPPER ID : ");
      for (int i = 0; i < 12; i++) {
        printf("0x%.8x ", cprbuf[ i ]);
      }
      printf("\n");
      B2FATAL("Unknown COPPER ID is found. CPRID = " << hex << subsysid << " Please check. Exiting...");
      exit(1);
      // StoreArray<RawCOPPER> ary;
      // (ary.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
    }
    //    delete[] cprbuf;
  }

  if (store_time_flag != 1) {
    B2FATAL("No time information could be extracted from Data. That should not happen. Exiting...");
  }
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();
  evtmetadata->setExperiment(sndhdr.GetExpNum());
  evtmetadata->setRun(sndhdr.GetRunNum());
  evtmetadata->setSubrun(sndhdr.GetSubRunNum());
  evtmetadata->setEvent(sndhdr.GetEventNumber());
  evtmetadata->setTime(mtime);  //time(NULL));
  //  evtmetadata->setTime((unsigned long long int) utime);//time(NULL));

  if (error_flag) setErrorFlag(error_flag, evtmetadata);

  delete[] evtbuf;

  //  B2INFO("Raw2Ds: DataStore Restored!!");
  return;
}

void Raw2DsModule::endRun()
{
  //fill Run data

  B2INFO("Raw2Ds: endRun done.");
}


void Raw2DsModule::terminate()
{
  B2INFO("Raw2Ds: terminate called");
}

void Raw2DsModule::setErrorFlag(unsigned int error_flag, StoreObjPtr<EventMetaData> evtmetadata)
{
  //  RawHeader_latest raw_hdr;
  int error_set = 0;
  if (error_flag & RawHeader_latest::B2LINK_PACKET_CRC_ERROR) {
    evtmetadata->addErrorFlag(EventMetaData::c_B2LinkPacketCRCError);
    error_set = 1;
  }
  if (error_flag & RawHeader_latest::B2LINK_EVENT_CRC_ERROR) {
    evtmetadata->addErrorFlag(EventMetaData::c_B2LinkEventCRCError);
    error_set = 1;
  }
  if (error_set)  B2INFO("Raw2Ds: Error flag was set in EventMetaData.");
}
