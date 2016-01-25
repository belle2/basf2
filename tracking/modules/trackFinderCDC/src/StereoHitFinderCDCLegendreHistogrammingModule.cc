#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

StereoHitFinderCDCLegendreHistogrammingModule::StereoHitFinderCDCLegendreHistogrammingModule() :
  TrackFinderCDCBaseModule()
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  ModuleParamList moduleParamList = this->getParamList();
  m_stereohitsCollector.exposeParameters(&moduleParamList);

  addParam("useOldImplementation", m_param_useOldImplementation, "Use the old implementation.", m_param_useOldImplementation);
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  m_stereohitsCollector.initialize();
  m_stereohitsProcessor.initialize();
  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  m_stereohitsCollector.terminate();
  m_stereohitsProcessor.terminate();
  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  if (m_param_useOldImplementation) {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcessor.addStereoHitsWithQuadTree(track);
      m_stereohitsProcessor.postprocessTrack(track);
    }
  } else {
    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    const std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
    std::vector<CDCRLTaggedWireHit> rlTaggedWireHits;
    rlTaggedWireHits.reserve(2 * wireHits.size());
    for (const CDCWireHit& wireHit : wireHits) {
      for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
        rlTaggedWireHits.emplace_back(&wireHit, rlInfo);
      }
    }

    m_stereohitsCollector.collect(tracks, rlTaggedWireHits);

    for (CDCTrack& track : tracks) {
      const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

      track.shiftToPositiveArcLengths2D();
      track.sortByArcLength2D();

      const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
      CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
      track.setStartTrajectory3D(newStartTrajectory);
    }
  }
}
