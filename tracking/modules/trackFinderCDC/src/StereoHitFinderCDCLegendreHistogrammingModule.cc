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

  addParam("useBestMatch", m_param_useBestMatch, "Use the best-match algorithm instead of the first match.", m_param_useBestMatch);

  ModuleParamList paramList = this->getParamList();
  m_stereohitsCollectorBestMatch.exposeParameters(&paramList, "bestMatch");
  m_stereohitsCollectorFirstMatch.exposeParameters(&paramList, "firstMatch");
  setParamList(paramList);
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  if (m_param_useBestMatch) {
    m_stereohitsCollectorBestMatch.initialize();
  } else {
    m_stereohitsCollectorFirstMatch.initialize();
  }

  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  if (m_param_useBestMatch) {
    m_stereohitsCollectorBestMatch.terminate();
  } else {
    m_stereohitsCollectorFirstMatch.terminate();
  }

  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  // Initialize the RL hits
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const auto& wireHits = wireHitTopology.getWireHits();
  std::vector<CDCRLTaggedWireHit> rlTaggedWireHits;
  rlTaggedWireHits.reserve(2 * wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
      rlTaggedWireHits.emplace_back(&wireHit, rlInfo);
    }
  }

  // Collect the hits for each track
  if (m_param_useBestMatch) {
    m_stereohitsCollectorBestMatch.collect(tracks, rlTaggedWireHits);
  } else {
    m_stereohitsCollectorFirstMatch.collect(tracks, rlTaggedWireHits);
  }

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
