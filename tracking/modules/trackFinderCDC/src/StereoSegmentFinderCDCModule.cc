#include <tracking/modules/trackFinderCDC/StereoSegmentFinderCDCModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoSegmentFinderCDC)

StereoSegmentFinderCDCModule::StereoSegmentFinderCDCModule() :
  TrackFinderCDCFromSegmentsModule()
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  addParam("useQuadTree", m_param_useQuadTree, "Whether to use the quad tree implementation or not.", m_param_useQuadTree);

  ModuleParamList paramList = this->getParamList();
  m_collectorFilter.exposeParameters(&paramList, "filter");
  m_collectorQuadTree.exposeParameters(&paramList, "quadTree");
  setParamList(paramList);
}

/** Initialize the stereo quad trees */
void StereoSegmentFinderCDCModule::initialize()
{
  if (m_param_useQuadTree) {
    m_collectorQuadTree.initialize();
  } else {
    m_collectorFilter.initialize();
  }

  TrackFinderCDCFromSegmentsModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoSegmentFinderCDCModule::terminate()
{
  if (m_param_useQuadTree) {
    m_collectorQuadTree.terminate();
  } else {
    m_collectorFilter.terminate();
  }

  TrackFinderCDCFromSegmentsModule::terminate();
}

void StereoSegmentFinderCDCModule::generate(std::vector<TrackFindingCDC::CDCSegment2D>& segments,
                                            std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  if (m_param_useQuadTree) {
    m_collectorQuadTree.collect(tracks, segments);
  } else {
    m_collectorFilter.collect(tracks, segments);
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

  // Delete all taken segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCSegment2D & segment) {
    return segment.getAutomatonCell().hasTakenFlag() or segment.isFullyTaken();
  }), segments.end());
}
