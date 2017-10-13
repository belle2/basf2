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
    "Module that returns true, if the trigger condition "
    "for the 2017 cosmic runs is fulfilled.\n"
    "trigger conditions:\n"
    "  with back-to-back: two back-to-back track segments in superlayer 2 "
    "plus one ECL hit\n"
    "  without back-to-back: on track segment in superlayer 2 "
    "plus one ECL hit"
  );

  addParam("tsHitCollectionName", m_tsHitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("BackToBack", m_backToBack,
           "Switch to turn back-to-back requirement on or off.",
           true);
  addParam("skipECL", m_skipECL,
           "Switch to turn off the ECL part of the cosmic trigger.",
           false);
}

void
TRGGDLCosmicRunModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required(m_tsHitCollectionName);
  if (!m_skipECL) StoreArray<TRGECLTrg>::required();
}

void
TRGGDLCosmicRunModule::event()
{
  StoreArray<CDCTriggerSegmentHit> tshits(m_tsHitCollectionName);
  bool TSinMerger[12] = {false};
  bool TSinSL2 = false;
  for (int its = 0; its < tshits.getEntries(); ++its) {
    if (tshits[its]->getISuperLayer() == 2) {
      // SegmentID in SuperLayer 2 starts at 320
      // One merger corresponds to 16 segments
      unsigned mergerID = (tshits[its]->getSegmentID() - 320) / 16;
      TSinMerger[mergerID] = true;
      TSinSL2 = true;
    }
  }
  bool BackToBack = false;
  for (unsigned i = 0; i < 6; ++i) {
    BackToBack |= (TSinMerger[i] && TSinMerger[i + 6]);
  }

  bool TCHit = false;
  if (!m_skipECL) {
    StoreArray<TRGECLTrg> tchit;
    for (int itchit  = 0; itchit < tchit.getEntries(); itchit++) {
      if ((tchit[itchit] -> getNofTCHit()) > 0) {
        TCHit = true;
      }
    }
  }

  if (m_backToBack)
    setReturnValue(BackToBack && (TCHit || m_skipECL));
  else
    setReturnValue(TSinSL2 && (TCHit || m_skipECL));
}
