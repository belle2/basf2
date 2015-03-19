#include <tracking/modules/trackFinderOutputCombiner/SegmentQualityCheckModule.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFinderOutputCombiner/FittingMatrix.h>

#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentQualityCheck);

SegmentQualityCheckModule::SegmentQualityCheckModule()
{
  addParam("MaximalMeanOfDriftLength",
           m_param_maximalMeanOfDriftLength,
           "Maximal mean of the drift length for a segment to be used later.",
           0.8);
  addParam("RecoSegments",
           m_param_recoSegments,
           "Name of the Store Array for the segments from the local track finder.");
}


void SegmentQualityCheckModule::initialize()
{
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> recoSegments(m_param_recoSegments);
  recoSegments.isRequired();
}

void SegmentQualityCheckModule::event()
{
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_recoSegments);
  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;
  std::vector<CDCRecoSegment2D> goodRecoSegments;
  goodRecoSegments.reserve(recoSegments.size());

  for (const CDCRecoSegment2D& segment : recoSegments) {
    if (isGoodSegment(segment)) {
      goodRecoSegments.push_back(segment);
    }
  }

  recoSegments.clear();
  recoSegments.reserve(goodRecoSegments.size());

  recoSegments.insert(recoSegments.end(), goodRecoSegments.begin(), goodRecoSegments.end());
}

bool SegmentQualityCheckModule::isGoodSegment(const CDCRecoSegment2D& segment)
{

  double meanOfDriftLengths = 0;
  bool hasOnlyOneLayer = true;
  ILayerType firstLayer = segment[0].getWire().getILayer();

  for (const CDCRecoHit2D& recoHit2D : segment) {
    meanOfDriftLengths += recoHit2D.getRefDriftLength();
    if (recoHit2D.getWire().getILayer() != firstLayer)
      hasOnlyOneLayer = false;
  }

  meanOfDriftLengths /= segment.size();
  bool looksLikeBackground = (meanOfDriftLengths > m_param_maximalMeanOfDriftLength or hasOnlyOneLayer);
  bool isTooSmall = segment.size() < 3;

  return not looksLikeBackground and not isTooSmall;
}
