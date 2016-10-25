#include "trg/cdc/modules/trgcdc/CDCTriggerTSFModule.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerTSF);

CDCTriggerTSFModule::CDCTriggerTSFModule() : Module::Module()
{
  setDescription(
    "The Track Segment Finder module of the CDC trigger.\n"
    "Combines CDCHits from the same super layer to CDCTriggerSegmentHits.\n"
  );

  addParam("InnerTSLUTFile",
           m_innerTSLUTFilename,
           "The filename of LUT for track segments from the inner-most super layer",
           string(""));
  addParam("OuterTSLUTFile",
           m_outerTSLUTFilename,
           "The filename of LUT for track segments from the outer super layers",
           string(""));
}

void
CDCTriggerTSFModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerSegmentHit>::registerPersistent();
  StoreArray<CDCHit>::required();
  // register relations
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  StoreArray<CDCHit> cdcHits;
  StoreArray<MCParticle> mcparticles;
  segmentHits.registerRelationTo(cdcHits);
  mcparticles.registerRelationTo(segmentHits);

  // prepare track segment shapes
  // ... TODO ...
}

void
CDCTriggerTSFModule::event()
{
  StoreArray<CDCHit> cdchits;

  // fill CDCHits into track segment shapes
  // ... TODO ...

  // simulate track segments and create track segment hits
  // ... TODO ...
}
