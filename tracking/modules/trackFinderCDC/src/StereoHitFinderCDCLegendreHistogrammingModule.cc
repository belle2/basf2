#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCLegendreHistogrammingModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCLegendreHistogramming)

StereoHitFinderCDCLegendreHistogrammingModule::StereoHitFinderCDCLegendreHistogrammingModule() :
  TrackFinderCDCBaseModule()
{
  setDescription("Tries to add CDC stereo hits to the found CDC tracks by applying a histogramming method with a quad tree.");

  addParam("quadTreeLevel",
           m_stereohitsProcesser.getQuadTreeLevelParameter(),
           "The number of levels for the quad tree search.",
           m_stereohitsProcesser.getQuadTreeLevelValue());

  addParam("minimumNumberOfHits",
           m_stereohitsProcesser.getMinimumNumberOfHitsParameter(),
           "The minimum number of hits in a quad tree bin to be called as result.",
           m_stereohitsProcesser.getMinimumNumberOfHitsValue());

  addParam("useSegments",
           m_param_useSegments,
           "Whether to use the segments or the hits in the quad tree.",
           false);

  addParam("checkForB2BTracks",
           m_stereohitsProcesser.getCheckForB2BTracksParameter(),
           "Set to false to skip the check for back-2-back tracks (good for cosmics)",
           m_stereohitsProcesser.getCheckForB2BTracksValue());
}

/** Initialize the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::initialize()
{
  m_stereohitsProcesser.initialize();
  TrackFinderCDCBaseModule::initialize();
}

/** Terminate the stereo quad trees */
void StereoHitFinderCDCLegendreHistogrammingModule::terminate()
{
  m_stereohitsProcesser.terminate();
  TrackFinderCDCBaseModule::terminate();
}

void StereoHitFinderCDCLegendreHistogrammingModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  if (m_param_useSegments) {
    StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments("CDCRecoSegment2DVector");
    std::vector<CDCRecoSegment2D>& segments = *storedRecoSegments;

    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser.addStereoHitsWithQuadTree(track, segments);
      m_stereohitsProcesser.postprocessTrack(track);
    }
  } else {
    for (CDCTrack& track : tracks) {
      m_stereohitsProcesser.addStereoHitsWithQuadTree(track);
      m_stereohitsProcesser.postprocessTrack(track);
    }
  }
}
