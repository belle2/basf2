//+
// File : Convert2RawDet.cc
// Description : Module to convert from RawCOPPER or RawDataBlock to RawDetector objects
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 24 - Oct - 2014
//-
#include <rawdata/modules/Convert2RawDet.h>
#include <TSystem.h>
#include <stdlib.h>

#include "framework/datastore/StoreObjPtr.h"

// #define DESY

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Convert2RawDet)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Convert2RawDetModule::Convert2RawDetModule() : Module()
{
  //Set module properties
  setDescription("convert from RawCOPPER or RawDataBlock to RawDetector objects");
  setPropertyFlags(c_Input);
  m_nevt = 0;
}


Convert2RawDetModule::~Convert2RawDetModule()
{
}

void Convert2RawDetModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Initialize EvtMetaData

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
  //  registerRawCOPPERs();
}


void Convert2RawDetModule::beginRun()
{
  B2INFO("beginRun called.");
}


void Convert2RawDetModule::event()
{
  //  B2INFO("Event " << m_nevt);

  StoreArray<RawDataBlock> raw_datablkarray;
  for (int i = 0; i < raw_datablkarray.getEntries(); i++) {

    convertDataObject(raw_datablkarray[ i ]);
  }
  raw_datablkarray.clear();


  StoreArray<RawCOPPER> raw_cprarray;
  for (int i = 0; i < raw_cprarray.getEntries(); i++) {
    //    (raw_cprarray[ i ]->GetBuffer(0))[ 6 ] |= 0x02000000;
    convertDataObject((RawDataBlock*)(raw_cprarray[ i ]));
  }
  raw_cprarray.clear();

  m_nevt++;
}


void Convert2RawDetModule::convertDataObject(RawDataBlock* raw_dblk)
{
  //
  // Convert RawDataBlock to Raw***
  //

  int num_eve = raw_dblk->GetNumEvents();
  if (num_eve != 1) {
    printf("[FATAL] %s : This module should be used for data with num_eve = 1\n", __PRETTY_FUNCTION__);
    exit(1);
  }

  int num_nodes = raw_dblk->GetNumNodes();


  //     {
  //     int nwords = raw_dblk->TotalBufNwords();
  //     int* temp_buf = new int[ nwords ];
  //     memcpy( temp_buf, raw_dblk->GetWholeBuffer(), nwords*sizeof(int) );
  //     StoreArray<RawCOPPER> ary;
  //     (ary.appendNew())->SetBuffer(temp_buf, nwords, 1, num_eve, num_nodes);
  //     }
  //     return;

  for (int j = 0; j < num_eve; j++) {
    for (int k = 0; k < num_nodes; k++) {
      // index of blkidries
      int blkid = j * num_nodes + k;

      // allocate a new buffer
      int nwords = raw_dblk->GetBlockNwords(blkid);
      int* temp_buf = new int[ nwords ];
      memcpy(temp_buf, raw_dblk->GetBuffer(blkid), nwords * sizeof(int));


      // Set buffer to RawCOPPER class to access detector ID
      RawCOPPER tempcpr;
      const int temp_num_eve = 1;
      const int temp_num_nodes = 1;
      int delete_flag;
      delete_flag = 0; // this buffer will not be deleted in RawCOPPER destructor.
      tempcpr.SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      int subsys_id = tempcpr.GetNodeID(0);



      delete_flag = 1; // this buffer will be deleted in Raw*** destructor.



      if ((subsys_id & DETECTOR_MASK) == SVD_ID) {
        StoreArray<RawSVD> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == CDC_ID) {
        StoreArray<RawCDC> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == TOP_ID) {
        StoreArray<RawTOP> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == ARICH_ID) {
        StoreArray<RawARICH> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == BECL_ID) {
        StoreArray<RawECL> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == EECL_ID) {
        StoreArray<RawECL> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == BKLM_ID) {
        StoreArray<RawKLM> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if ((subsys_id & DETECTOR_MASK) == EKLM_ID) {
        StoreArray<RawKLM> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else if (((subsys_id & DETECTOR_MASK) & 0xF0000000)  == TRGDATA_ID) {
        StoreArray<RawTRG> ary;
        (ary.appendNew())->SetBuffer(temp_buf, nwords, delete_flag, temp_num_eve, temp_num_nodes);
      } else {
        printf("[FATAL] Undefined detector ID(0x%.8x). Exiting...\n", subsys_id);
        exit(1);
      }



    }
  }


  return;
}


void Convert2RawDetModule::endRun()
{
  //fill Run data

  B2INFO("Convert2RawDet: endRun done.");
}


void Convert2RawDetModule::terminate()
{
  B2INFO("Convert2RawDet: terminate called");
}

