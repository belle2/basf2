#include "trg/gdl/modules/trggdl/TRGGDLCosmicRunModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(TRGGDLCosmicRun);

TRGGDLCosmicRunModule::TRGGDLCosmicRunModule() : Module::Module()
{
  setDescription(
    "Module that returns true, if the back-to-back condition "
    "of track segments in SL 2 is fulfilled (for 2017 cosmic test).\n"
  );

  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
}

void
TRGGDLCosmicRunModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required(m_hitCollectionName);
  StoreArray<TRGECLTrg>::registerPersistent();
}

void
TRGGDLCosmicRunModule::event()
{
  StoreArray<CDCTriggerSegmentHit> hits(m_hitCollectionName);
  bool TSinMerger[12] = {false};
  for (int ihit = 0; ihit < hits.getEntries(); ++ihit) {
    if (hits[ihit]->getISuperLayer() == 2) {
      // SegmentID in SuperLayer 2 starts at 320
      // One merger corresponds to 16 segments
      unsigned mergerID = (hits[ihit]->getSegmentID() - 320) / 16;
      TSinMerger[mergerID] = true;
    }
  }
  bool BackToBack = false;
  for (unsigned i = 0; i < 6; ++i) {
    BackToBack |= (TSinMerger[i] && TSinMerger[i + 6]);
  }

  bool TCHit = false;
  StoreArray<TRGECLTrg> tchit;
  for (int itchit  = 0; itchit < tchit.getEntries(); itchit++) {
    if ((tchit[itchit] -> getNofTCHit()) > 0) {
      TCHit = true;
    }

  }



  setReturnValue(BackToBack && TCHit);
}
