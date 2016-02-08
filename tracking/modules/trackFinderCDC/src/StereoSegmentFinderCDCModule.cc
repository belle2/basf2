#include <tracking/modules/trackFinderCDC/StereoSegmentFinderCDCModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoSegmentFinderCDC)

StereoSegmentFinderCDCModule::StereoSegmentFinderCDCModule() :
  TrackFinderCDCFromSegmentsModule()
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  ModuleParamList paramList = this->getParamList();
  m_collector.exposeParameters(&paramList);
  setParamList(paramList);
}

/** Initialize the stereo quad trees */
void StereoSegmentFinderCDCModule::initialize()
{
  m_collector.initialize();

  TrackFinderCDCFromSegmentsModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoSegmentFinderCDCModule::terminate()
{
  m_collector.terminate();

  TrackFinderCDCFromSegmentsModule::terminate();
}

void StereoSegmentFinderCDCModule::generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
                                            std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  m_collector.collect(tracks, segments);

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
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) {
    return segment.getAutomatonCell().hasTakenFlag() or segment.isFullyTaken();
  }), segments.end());
}
