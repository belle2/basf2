#include "trg/cdc/modules/fitting/CDCTrigger3DFitterModule.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/datastore/RelationVector.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTrigger3DFitter);

CDCTrigger3DFitterModule::CDCTrigger3DFitterModule() : Module::Module()
{
  setDescription(
    "The 3D fitter module of the CDC trigger.\n"
    "Selects stereo hits around a given 2D track and performs a linear fit "
    "in the s-z plane (s: 2D arclength).\n"
  );

  addParam("inputCollection", m_inputCollectionName,
           "Name of the StoreArray holding the input tracks from the 2D fitter.",
           string("Trg2DFitterTracks"));
  addParam("outputCollection", m_outputCollectionName,
           "Name of the StoreArray holding the 3D output tracks.",
           string("Trg3DFitterTracks"));
}

void
CDCTrigger3DFitterModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerTrack>::registerPersistent(m_outputCollectionName);
  StoreArray<CDCTriggerTrack>::required(m_inputCollectionName);
  StoreArray<CDCTriggerSegmentHit>::required();
  // register relations
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracks3D(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  tracks3D.registerRelationTo(tracks2D);
  tracks3D.registerRelationTo(segmentHits);
}

void
CDCTrigger3DFitterModule::event()
{
  StoreArray<CDCTriggerTrack> tracks2D(m_inputCollectionName);
  StoreArray<CDCTriggerTrack> tracks3D(m_outputCollectionName);
  StoreArray<CDCTriggerSegmentHit> hits;

  for (int itrack = 0; itrack < tracks2D.getEntries(); ++itrack) {
    // select stereo hits
    // ... TODO ...

    // do the fit and create a new track
    // ... TODO ...
  }
}
