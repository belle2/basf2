#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>
#include <tracking/trackFindingCDC/legendre/stereohits/StereohitsProcesser.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

StereoHitFinderCDCLegendreHistogrammingModule::StereoHitFinderCDCLegendreHistogrammingModule() :
  TrackFinderCDCBaseModule(), m_stereohitsProcesser(nullptr)
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  addParam("debugOutput",
           m_param_debugOutput,
           "Flag to turn on debug output.",
           false);

  addParam("quadTreeLevel",
           m_param_quadTreeLevel,
           "The number of levels for the quad tree search.",
           static_cast<unsigned int>(5));

  addParam("minimumHitsInQuadtree",
           m_param_minimumHitsInQuadTree,
           "The minimum number of hits in a quad tree bin to be called as result.",
           static_cast<unsigned int>(5));

  addParam("useOldImplementation",
           m_param_useOldImplementation,
           "Whether to use the old implementation of the quad tree.",
           false);

  addParam("useSegments",
           m_param_useSegments,
           "Whether to use the segments or the hits in the quad tree.",
           false);

  addParam("checkForB2BTracks", m_param_checkForB2BTracks,
           "Set to false to skip the check for back-2-back tracks (good for cosmics)",
           true);
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  m_stereohitsProcesser = new TrackFindingCDC::StereohitsProcesser(m_param_quadTreeLevel, m_param_debugOutput,
      m_param_checkForB2BTracks);

  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  delete m_stereohitsProcesser;

  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{

  if (m_param_useSegments) {
    StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments("CDCRecoSegment2DVector");
    std::vector<CDCRecoSegment2D>& segments = *storedRecoSegments;

    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser->makeHistogrammingWithSegments(track, segments, m_param_minimumHitsInQuadTree);
    }
  } else {
    if (m_param_useOldImplementation) {
      for (CDCTrack& track : tracks) {
        m_stereohitsProcesser->makeHistogramming(track, m_param_minimumHitsInQuadTree);

      }
    } else {
      for (CDCTrack& track : tracks) {
        m_stereohitsProcesser->makeHistogrammingWithNewQuadTree(track, m_param_minimumHitsInQuadTree);
      }
    }
  }

  // Postprocessing: sort by arc length, set all arc lengths to be positive and fit the tracks.
  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();
  for (CDCTrack& track : tracks) {
    track.shiftToPositiveArcLengths2D();
    track.sortByArcLength2D();

    const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
    CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
    track.setStartTrajectory3D(newStartTrajectory);
  }
}
