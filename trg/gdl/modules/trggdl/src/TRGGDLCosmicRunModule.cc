#include "trg/gdl/modules/trggdl/TRGGDLCosmicRunModule.h"

#include <framework/datastore/DataStore.h>

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
  m_segmentHits.isRequired(m_tsHitCollectionName);
  if (!m_skipECL) m_tchit.isRequired();
}

void
TRGGDLCosmicRunModule::event()
{
  bool TSinMerger[12] = {false};
  bool TSinSL2 = false;
  for (int its = 0; its < m_segmentHits.getEntries(); ++its) {
    if (m_segmentHits[its]->getISuperLayer() == 2) {
      // SegmentID in SuperLayer 2 starts at 320
      // One merger corresponds to 16 segments
      unsigned mergerID = (m_segmentHits[its]->getSegmentID() - 320) / 16;
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
    for (int itchit  = 0; itchit < m_tchit.getEntries(); itchit++) {
      if ((m_tchit[itchit] -> getNofTCHit()) > 0) {
        TCHit = true;
      }
    }
  }

  if (m_backToBack)
    setReturnValue(BackToBack && (TCHit || m_skipECL));
  else
    setReturnValue(TSinSL2 && (TCHit || m_skipECL));
}
