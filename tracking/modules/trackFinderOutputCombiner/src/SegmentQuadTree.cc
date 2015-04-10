#include <tracking/modules/trackFinderOutputCombiner/SegmentQuadTree.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentQuadTree);

SegmentQuadTreeModule::SegmentQuadTreeModule() : TrackFinderCDCFromSegmentsModule()
{
  addParam("Level", m_param_level, "Maximum Level for QuadTreeSearch.", static_cast<unsigned int>(10));
  addParam("MinimumItems", m_param_minimumItems, "Minimum number of hits in one QuadTreeCell.", static_cast<unsigned int>(10));
}

void SegmentQuadTreeModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  quadTreeSearch(segments, tracks);
}

void addHitToTrack(CDCTrack& track, const CDCRecoHit2D& hit, const CDCTrajectory3D& trajectory3D)
{
  const CDCRLWireHit* ptrRLWireHit = &(hit.getRLWireHit());
  Vector3D recoPos3D(hit.getRecoPos2D(), 0.0);
  FloatType perpS = trajectory3D.calcPerpS(recoPos3D);
  track.push_back(CDCRecoHit3D(ptrRLWireHit, recoPos3D, perpS));
}

void SegmentQuadTreeModule::quadTreeSearch(std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& tracks)
{
  StoreWrappedObjPtr< std::vector<CDCRecoFacet> > storedFacets("CDCRecoFacetVector");
  std::vector<CDCRecoFacet>& recoFacets = *storedFacets;

  Processor::ChildRanges ranges(Processor::rangeX(0, m_nbinsTheta),
                                Processor::rangeY(m_rMin, m_rMax));
  Processor qtProcessor(m_param_level, ranges);

  Processor::ReturnList hits_set;

  for (Processor::ItemType::TypeData& reco : recoSegments) {
    if (reco.getStereoType() == AXIAL) {
      hits_set.push_back(&reco);
    }
    if (reco.getAutomatonCell().hasTakenFlag()) {
      B2INFO("Yes")
    } else {
      B2INFO("No")
    }
  }

  qtProcessor.provideItemsSet(hits_set);

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  Processor::CandidateProcessorLambda lmdCandidateProcessing = [&](const Processor::ReturnList & items,
  Processor::QuadTree * node) -> void {
    B2DEBUG(90, "Found track with " << items.size() << " on level " << static_cast<unsigned int>(node->getLevel()))
    tracks.emplace_back();
    CDCTrack& foundTrack = tracks.back();

    CDCObservations2D observations;
    CDCTrajectory2D trajectory;

    for (Processor::ItemType::TypeData* item : items)
    {
      observations.append(*item);
    }

    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    fitter.update(trajectory, observations);
    //trajectory.setLocalOrigin(items.front()->getStartRecoPos2D());
    trajectory.setLocalOrigin(items.front()->getFrontRecoPos2D());

    CDCTrajectory3D trajectory3D(trajectory, CDCTrajectorySZ::basicAssumption());

    foundTrack.setStartTrajectory3D(trajectory3D);

    for (Processor::ItemType::TypeData* recoSegment : items)
    {
      /*addHitToTrack(foundTrack, recoSegment->getStartRecoHit2D(), trajectory3D);
      addHitToTrack(foundTrack, recoSegment->getMiddleRecoHit2D(), trajectory3D);
      addHitToTrack(foundTrack, recoSegment->getEndRecoHit2D(), trajectory3D);*/
      for (const CDCRecoHit2D& recoHit : recoSegment->items()) {
        addHitToTrack(foundTrack, recoHit, trajectory3D);
      }
    }
  };

  qtProcessor.fillGivenTree(lmdCandidateProcessing, m_param_minimumItems);

  B2DEBUG(90, "Found " << tracks.size() << " tracks")
}

void SegmentQuadTreeModule::printQuadTree(Processor::QuadTree* node)
{
  Processor::QuadTree::Children* children = node->getChildren();

  unsigned int nItems = node->getNItems();
  unsigned int usedItems = 0;

  for (Processor::ItemType* item : node->getItemsVector()) {
    if (item->isUsed()) {
      usedItems++;
    }
  }

  B2DEBUG(110, static_cast<unsigned int>(node->getLevel()) << " (" << node->getXMin() << ", " << node->getXMax() << ") - (" <<
          node->getYMin() << ", " << node->getYMax() << "): " << usedItems << "/" << nItems);

  if (children != nullptr) {
    printQuadTree(children->get(0, 0));
    printQuadTree(children->get(0, 1));
    printQuadTree(children->get(1, 0));
    printQuadTree(children->get(1, 1));
  }

}
