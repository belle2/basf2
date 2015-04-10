#include <tracking/modules/trackFinderOutputCombiner/SegmentQuadTree.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentQuadTree);

void SegmentQuadTreeModule::addHitToTrack(CDCTrack& track, const CDCRecoHit2D& hit, const CDCTrajectory3D& trajectory3D)
{
  const CDCRLWireHit* ptrRLWireHit = &(hit.getRLWireHit());
  Vector3D recoPos3D(hit.getRecoPos2D(), 0.0);
  FloatType perpS = trajectory3D.calcPerpS(recoPos3D);
  track.push_back(CDCRecoHit3D(ptrRLWireHit, recoPos3D, perpS));
}

SegmentQuadTreeModule::SegmentQuadTreeModule() : TrackFinderCDCFromSegmentsModule()
{
  addParam("Level", m_param_level, "Maximum Level for QuadTreeSearch.", static_cast<unsigned int>(10));
  addParam("MinimumItems", m_param_minimumItems, "Minimum number of hits in one QuadTreeCell.", static_cast<unsigned int>(10));
}

void SegmentQuadTreeModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  quadTreeSearch(segments, tracks);
}

void SegmentQuadTreeModule::quadTreeSearch(std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& tracks)
{
  typedef std::vector<CDCRecoSegment2D*> RecoVector;
  std::vector<RecoVector> foundTracks;

  Processor::ChildRanges ranges(Processor::rangeX(0, m_nbinsTheta),
                                Processor::rangeY(m_rMin, m_rMax));

  Processor::ReturnList hits_set;

  for (Processor::ItemType::TypeData& reco : recoSegments) {
    if (reco.getStereoType() == AXIAL) {
      hits_set.push_back(&reco);
    }
  }

  Processor::CandidateProcessorLambda lmdCandidateProcessing = [&](const Processor::ReturnList & items,
  Processor::QuadTree * node) -> void {
    B2DEBUG(90, "Found track with " << items.size() << " on level " << static_cast<unsigned int>(node->getLevel()))
    foundTracks.push_back(items);
  };

  Processor qtProcessor(m_param_level, ranges, Vector2D(0, 0));
  qtProcessor.provideItemsSet(hits_set);
  qtProcessor.fillGivenTree(lmdCandidateProcessing, m_param_minimumItems);

  B2DEBUG(90, "Found " << foundTracks.size() << " tracks")

  for (const RecoVector& track : foundTracks) {
    tracks.emplace_back();
    CDCTrack& foundTrack = tracks.back();
    CDCObservations2D observations;
    CDCTrajectory2D trajectory;

    for (Processor::ItemType::TypeData* item : track) {
      observations.append(*item);
    }

    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    fitter.update(trajectory, observations);
    //trajectory.setLocalOrigin(items.front()->getStartRecoPos2D());
    trajectory.setLocalOrigin(track.front()->getFrontRecoPos2D());

    CDCTrajectory3D trajectory3D(trajectory, CDCTrajectorySZ::basicAssumption());

    foundTrack.setStartTrajectory3D(trajectory3D);

    for (Processor::ItemType::TypeData* recoSegment : track) {
      for (const CDCRecoHit2D& recoHit : recoSegment->items()) {
        addHitToTrack(foundTrack, recoHit, trajectory3D);
      }
    }
  }

}
