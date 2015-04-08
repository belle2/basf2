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

SegmentQuadTreeModule::SegmentQuadTreeModule() : TrackFinderCDCFromSegmentsModule(), m_quadTree(0, m_nbinsTheta, m_rMin, m_rMax, 0,
      nullptr)
{
  addParam("Level", m_param_level, "Maximum Level for QuadTreeSearch.", static_cast<unsigned int>(10));
  addParam("MinimumItems", m_param_minimumItems, "Minimum number of hits in one QuadTreeCell.", static_cast<unsigned int>(10));
}

void SegmentQuadTreeModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  quadTreeSearch(segments, tracks);

  printDebugInformation();
}

void SegmentQuadTreeModule::quadTreeSearch(std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& tracks)
{
  QuadTreeProcessorSegments qtProcessor(m_param_level);

  std::vector<CDCRecoSegment2D*> hits_set;

  for (CDCRecoSegment2D& recoSegment : recoSegments) {
    if (recoSegment.getStereoType() == AXIAL) {
      hits_set.push_back(&recoSegment);
    }
  }

  qtProcessor.provideItemsSet(m_quadTree, hits_set);

  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  SegmentQuadTree::CandidateProcessorLambda lmdCandidateProcessing = [&](SegmentQuadTree * qt) -> void {
    B2DEBUG(90, "Found track with " << qt->getNItems() << " on level " << static_cast<unsigned int>(qt->getLevel()))
    tracks.emplace_back();
    CDCTrack& foundTrack = tracks.back();

    CDCObservations2D observations;
    CDCTrajectory2D trajectory;

    for (QuadTreeItem<CDCRecoSegment2D>*& quadTreeItem : qt->getItemsVector())
    {
      quadTreeItem->setUsedFlag();
      CDCRecoSegment2D* recoSegment = quadTreeItem->getPointer();
      observations.append(*recoSegment);
    }

    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    fitter.update(trajectory, observations);

    CDCTrajectory3D trajectory3D(trajectory, CDCTrajectorySZ::basicAssumption());

    foundTrack.setStartTrajectory3D(trajectory3D);

    for (QuadTreeItem<CDCRecoSegment2D>*& quadTreeItem : qt->getItemsVector())
    {
      CDCRecoSegment2D* recoSegment = quadTreeItem->getPointer();
      for (const CDCRecoHit2D& recoHit2D : recoSegment->items()) {
        const CDCRLWireHit* ptrRLWireHit = &(recoHit2D.getRLWireHit());
        Vector3D recoPos3D(recoHit2D.getRecoPos2D(), 0.0);
        FloatType perpS = trajectory3D.calcPerpS(recoPos3D);
        foundTrack.push_back(CDCRecoHit3D(ptrRLWireHit, recoPos3D, perpS));
      }
    }
  };

  qtProcessor.fillGivenTree(m_quadTree, lmdCandidateProcessing, m_param_minimumItems);

  qtProcessor.clear(m_quadTree);
}

void SegmentQuadTreeModule::printQuadTree(SegmentQuadTree* node)
{
  SegmentQuadTree::Children* children = node->getChildren();

  unsigned int nItems = node->getNItems();
  unsigned int usedItems = 0;

  for (QuadTreeItem<CDCRecoSegment2D>* item : node->getItemsVector()) {
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

void SegmentQuadTreeModule::printDebugInformation()
{
  printQuadTree(&m_quadTree);
}
