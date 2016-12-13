#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

StereoHitFinderCDCLegendreHistogrammingModule::StereoHitFinderCDCLegendreHistogrammingModule() :
  TrackFinderCDCBaseModule()
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  ModuleParamList paramList = this->getParamList();
  m_stereohitsCollector.exposeParameters(&paramList, "singleMatch");
  setParamList(paramList);
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  m_stereohitsCollector.initialize();
  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  m_stereohitsCollector.terminate();
  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  // Initialize the RL hits
  StoreWrappedObjPtr<std::vector< CDCWireHit>> storedWireHits("CDCWireHitVector");
  const std::vector<CDCWireHit>& wireHits = *storedWireHits;
  std::vector<CDCRLWireHit> rlTaggedWireHits;
  rlTaggedWireHits.reserve(2 * wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit.getAutomatonCell().hasTakenFlag() and not wireHit.isAxial()) {
      for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
        rlTaggedWireHits.emplace_back(&wireHit, rlInfo, wireHit.getRefDriftLength());
      }
    }
  }

  // Collect the hits for each track
  m_stereohitsCollector.collect(tracks, rlTaggedWireHits);

  // Postprocess each track (=fit)
  for (CDCTrack& track : tracks) {
    const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

    track.shiftToPositiveArcLengths2D();
    track.sortByArcLength2D();

    const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
    CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
    track.setStartTrajectory3D(newStartTrajectory);
  }
}
