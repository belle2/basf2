#include "trg/cdc/modules/fitting/CDCTrigger2DFitterModule.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/datastore/RelationVector.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTrigger2DFitter);

CDCTrigger2DFitterModule::CDCTrigger2DFitterModule() : Module::Module()
{
  setDescription(
    "The 2D fitter module of the CDC trigger.\n"
    "Performs a circle fit on a given set of axial CDCTriggerSegmentHits.\n"
    "Requires a preceding track finder to sort hits to tracks.\n"
  );

  addParam("inputCollection", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the track finder.",
           string("Trg2DFinderTracks"));
  addParam("outputCollection", m_outputCollectionName,
           "Name of the StoreArray holding the fitted output tracks.",
           string("Trg2DFitterTracks"));
}

void
CDCTrigger2DFitterModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  StoreArray<CDCTriggerSegmentHit>::required();
  // register relations
  StoreArray<CDCTriggerTrack> finderTracks(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> fitterTracks(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  fitterTracks.registerRelationTo(finderTracks);
  fitterTracks.registerRelationTo(segmentHits);
}

void
CDCTrigger2DFitterModule::event()
{
  StoreArray<CDCTriggerTrack> finderTracks(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> fitterTracks(m_outputCollectionName);

  for (int itrack = 0; itrack < finderTracks.getEntries(); ++itrack) {
    // get selected hits (positive relation weight)
    RelationVector<CDCTriggerSegmentHit> hits =
      finderTracks[itrack]->getRelationsTo<CDCTriggerSegmentHit>();
    for (unsigned ihit = 0; ihit < hits.size(); ++ihit) {
      if (hits.weight(ihit) > 0) {
        // ... TODO ...
      }
    }

    // do the fit and create a new track
    // ... TODO ...
  }
}
