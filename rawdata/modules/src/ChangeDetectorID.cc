//+
// File : ChangeDetectorID.cc
// Description : MModify Detector ID in RawCOPPER's header
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 24 - Oct - 2014
//-
#include <rawdata/modules/ChangeDetectorID.h>
#include <TSystem.h>
#include <stdlib.h>

#include "framework/datastore/StoreObjPtr.h"

// #define DESY

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ChangeDetectorID)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ChangeDetectorIDModule::ChangeDetectorIDModule() : Convert2RawDetModule()
{
  m_new_detector_id = 0;
  setDescription("Modify Detector ID");
  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NewDetectorID", m_new_detector_id, "new Detector ID");

}


ChangeDetectorIDModule::~ChangeDetectorIDModule()
{
}


void ChangeDetectorIDModule::event()
{

  StoreArray<RawDataBlock> raw_datablkarray;
  for (int i = 0; i < raw_datablkarray.getEntries(); i++) {
    convertDataObject(raw_datablkarray[ i ]);
  }
  raw_datablkarray.clear();

  StoreArray<RawCOPPER> raw_cprarray;
  for (int i = 0; i < raw_cprarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_cprarray[ i ]));
  }
  raw_cprarray.clear();

  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_svdarray[ i ]));
  }
  raw_svdarray.clear();

  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_cdcarray[ i ]));
  }
  raw_cdcarray.clear();

  StoreArray<RawTOP> raw_toparray;
  for (int i = 0; i < raw_toparray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_toparray[ i ]));
  }
  raw_toparray.clear();

  StoreArray<RawARICH> raw_aricharray;
  for (int i = 0; i < raw_aricharray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_aricharray[ i ]));
  }
  raw_aricharray.clear();

  StoreArray<RawECL> raw_eclarray;
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_eclarray[ i ]));
  }
  raw_eclarray.clear();

  StoreArray<RawKLM> raw_klmarray;
  for (int i = 0; i < raw_klmarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_klmarray[ i ]));
  }
  raw_klmarray.clear();

  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    convertDataObject((RawDataBlock*)(raw_trgarray[ i ]));
  }
  raw_trgarray.clear();

  m_nevt++;

}
void ChangeDetectorIDModule::convertDataObject(RawDataBlock* raw_dblk)
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
      unsigned int subsys_id = tempcpr.GetNodeID(0);

      delete_flag = 1; // this buffer will be deleted in Raw*** destructor.

      //
      // Set a new Detector ID
      //
      subsys_id = (m_new_detector_id & DETECTOR_MASK) | (subsys_id & COPPERID_MASK);
      PostRawCOPPERFormat_latest post_cpr;
      post_cpr.tmp_header.SetBuffer(tempcpr.GetBuffer(0));
      post_cpr.tmp_header.SetNodeID(subsys_id);
      subsys_id = tempcpr.GetNodeID(0);

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
