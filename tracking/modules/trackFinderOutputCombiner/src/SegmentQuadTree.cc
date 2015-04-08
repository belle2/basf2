#include <tracking/modules/trackFinderOutputCombiner/SegmentQuadTree.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>

#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentQuadTree);

SegmentQuadTreeModule::SegmentQuadTreeModule() : m_quadTree(0, m_nbinsTheta, m_rMin, m_rMax, 0, nullptr)
{
  addParam("RecoSegments",
           m_param_recoSegments,
           "Name of the Store Array for the segments from the local track finder.");
}


void SegmentQuadTreeModule::initialize()
{
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> recoSegments(m_param_recoSegments);
  recoSegments.isRequired();
}

void SegmentQuadTreeModule::event()
{
  B2INFO("Starting event...")
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_recoSegments);
  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;

  quadTreeSearch(recoSegments);
}

void SegmentQuadTreeModule::quadTreeSearch(std::vector<CDCRecoSegment2D>& recoSegments)
{

  B2INFO("Starting quadtree search...")

  QuadTreeProcessorSegments qtProcessor(2);
  m_quadTree.clearTree();

  B2INFO("Cleared tree")

  std::set<QuadTreeItem<CDCRecoSegment2D>*> hits_set;

  for (CDCRecoSegment2D& recoSegment : recoSegments) {
    hits_set.insert(new QuadTreeItem<CDCRecoSegment2D>(&recoSegment));
  }

  B2INFO("Filled items set")

  m_quadTree.provideItemsSet(qtProcessor, hits_set);

  B2INFO("Provided items set")

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  std::vector<std::vector<CDCRecoSegment2D*>> foundTracks;

  SegmentQuadTree::CandidateProcessorLambda lmdCandidateProcessing = [&](SegmentQuadTree * qt) -> void {
    std::for_each(qt->getItemsVector().begin(), qt->getItemsVector().end(), [](QuadTreeItem<CDCRecoSegment2D>* segment)
    {
      segment->setUsedFlag();
    });
    B2INFO("Found track")
    foundTracks.emplace_back();
    std::vector<CDCRecoSegment2D*>& foundTrack = foundTracks.back();

    for (QuadTreeItem<CDCRecoSegment2D>*& quadTreeItem : qt->getItemsVector())
    {
      foundTrack.push_back(quadTreeItem->getPointer());
    }
  };

  qtProcessor.fillGivenTree(&m_quadTree, lmdCandidateProcessing, 50, 0.07);

  B2INFO("Found tracks: " << foundTracks.size())
}
