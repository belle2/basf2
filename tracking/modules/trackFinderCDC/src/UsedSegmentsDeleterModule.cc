#include <tracking/modules/trackFinderCDC/UsedSegmentsDeleterModule.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(UsedSegmentsDeleter)

void UsedSegmentsDeleterModule::initialize()
{
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> >::required(m_param_segmentsStoreObjName);
}


void UsedSegmentsDeleterModule::event()
{
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_segmentsStoreObjName);
  if (not storedRecoSegments) {
    B2WARNING("The segments have not been created.");
    return;
  }

  std::vector<CDCRecoSegment2D>& segments = *storedRecoSegments;

  // Delete all taken segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) {
    return segment.getAutomatonCell().hasTakenFlag() or segment.isFullyTaken();
  }), segments.end());
}


UsedSegmentsDeleterModule::UsedSegmentsDeleterModule()
{
  addParam("SegmentsStoreObjName",
           m_param_segmentsStoreObjName,
           "Name of the output StoreObjPtr of the segments to be handled by this module.",
           std::string("CDCRecoSegment2DVector"));
}
