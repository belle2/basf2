#include <tracking/modules/trackFinderCDC/UsedSegmentsDeleterModule.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(UsedSegmentsDeleter)

void UsedSegmentsDeleterModule::initialize()
{
  StoreWrappedObjPtr< std::vector<CDCSegment2D> >::required(m_param_segmentsStoreObjName);
}


void UsedSegmentsDeleterModule::event()
{
  StoreWrappedObjPtr< std::vector<CDCSegment2D> > storedSegments(m_param_segmentsStoreObjName);
  if (not storedSegments) {
    B2WARNING("The segments have not been created.");
    return;
  }

  std::vector<CDCSegment2D>& segments = *storedSegments;

  // Delete all taken segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCSegment2D & segment) {
    return segment.getAutomatonCell().hasTakenFlag() or segment.isFullyTaken();
  }), segments.end());
}


UsedSegmentsDeleterModule::UsedSegmentsDeleterModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("SegmentsStoreObjName",
           m_param_segmentsStoreObjName,
           "Name of the output StoreObjPtr of the segments to be handled by this module.",
           std::string("CDCSegment2DVector"));
}
