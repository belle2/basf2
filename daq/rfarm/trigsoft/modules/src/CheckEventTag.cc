//+
// File : CheckEventTag.cc
// Description : Module to Check Event Tag
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#include <TSystem.h>
#include <stdlib.h>

#include "daq/rfarm/trigsoft/modules/CheckEventTag.h"
#include "framework/datastore/StoreObjPtr.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CheckEventTag)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CheckEventTagModule::CheckEventTagModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  //  setPropertyFlags(c_Input | c_ParallelProcessingCertified);

  m_nevt = 0;

  //Parameter definition
  B2INFO("CheckEventTag: Constructor done.");
}


CheckEventTagModule::~CheckEventTagModule()
{
}

void CheckEventTagModule::initialize()
{
  B2INFO("CheckEventTag: initialized.");
}


void CheckEventTagModule::beginRun()
{
  B2INFO("CheckEventTag: beginRun called.");
}


void CheckEventTagModule::event()
{
  StoreObjPtr<EventMetaData> evtmeta;
  int expno = evtmeta->getExperiment();
  int runno = evtmeta->getRun();
  int evtno = evtmeta->getEvent();

  // Functions to be used:
  // GetTTUtime ( 0 )
  // GetTTCtimeTRGType(0)
  // 1. Get RawFTSW
  StoreArray<RawFTSW> ftsw;
  unsigned int ftswctime = ftsw[0]->GetTTUtime(0);
  unsigned int ftswutime = ftsw[0]->GetTTCtimeTRGType(0);

  // Loop oveer COPPERs
  unsigned int cprutime[MAXCPR], cprctime[MAXCPR];
  StoreArray<RawSVD> svd;
  int nsvd = svd.getEntries();
  //  printf ( "evt = %d, nsvd = %d\n", evtno, nsvd );
  for (int i = 0; i < nsvd; i++) {
    cprutime[i] = svd[i]->GetTTUtime(0);
    cprctime[i] = svd[i]->GetTTCtimeTRGType(0);
  }

  // Compare patters
  /*
  printf ( "***** Exp:%d Run:%d Evt:%d *****\n", expno, runno, evtno );
  printf ( "FTSW Utime = %8.8x, CPR1 = %8.8x, CPR2 = %8.8x\n",
     ftswutime, cprutime[0], cprutime[1] );
  printf ( "FTSW Ctime = %8.8x, CPR1 = %8.8x, CPR2 = %8.8x\n",
     ftswctime, cprctime[0], cprctime[1] );
  */
  // Reference = FTSW
  if (nsvd != 2)
    printf("ERROR! No. of svd is not 2 : Event = %d : Nsvd = %d\n", evtno, nsvd);
  for (int i = 0; i < nsvd; i++) {
    if (cprutime[i] != ftswutime)
      printf("ERROR! Inconsistent UTIME : Event=%d : FTSW = %8.8x, SVDCPR[%d] = %8.8x\n", evtno, ftswutime, i, cprutime[i]);
    if (cprctime[i] != ftswctime)
      printf("ERROR! Inconsistent CTIME : Event=%d : FTSW = %8.8x, SVDCPR[%d] = %8.8x\n", evtno, ftswctime, i, cprctime[i]);
  }

  return;
}

void CheckEventTagModule::endRun()
{
  B2INFO("CheckEventTag: endRun done.");
}


void CheckEventTagModule::terminate()
{
  B2INFO("CheckEventTag: terminate called")
}

