#include <tracking/trackFindingCDC/legendre/stereohits/StereohitsProcesser.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/legendre/quadtree/StereoHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


/** Returns a bool if the rlWire can be matched to a track. */
bool isValidHitTrajectoryMatch(const CDCRLWireHit& rlWireHit)
{
  if (rlWireHit.getStereoType() == StereoType::c_Axial or rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag())
    return false;


  // Check for number of layers in between

  return true;
}

void StereohitsProcesser::postprocessTrack(CDCTrack& track)
{
  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

  track.shiftToPositiveArcLengths2D();
  track.sortByArcLength2D();

  const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
  CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
  track.setStartTrajectory3D(newStartTrajectory);
}


void StereohitsProcesser::fillHitsVector(std::vector<const CDCRecoHit3D*>& hitsVector, const CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.getOuterExit().hasNAN();

  const auto& rlWireHits = wireHitTopology.getRLWireHits();
  hitsVector.reserve(rlWireHits.size());

  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (isValidHitTrajectoryMatch(rlWireHit)) {

      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      if (wire.isInCellZBounds(recoPos3D)) {
        double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
        if (perpS < 0) {
          if (isCurler) {
            perpS += 2 * TMath::Pi() * radius;
          } else if (m_param_checkForB2BTracks) {
            continue;
          }
        }
        CDCRecoHit3D* newRecoHit = new CDCRecoHit3D(rlWireHit, recoPos3D, perpS);
        hitsVector.push_back(newRecoHit);
      }
    }
  }
}

void StereohitsProcesser::addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<const CDCRecoHit3D*>& foundStereoHits,
    const std::vector<const CDCRecoHit3D*>& doubledRecoHits, const std::vector<const CDCRecoHit3D*>& hitsVector) const
{
  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
  foundStereoHits.end(), [&doubledRecoHits](const CDCRecoHit3D * recoHit3D) -> bool {
    return std::find(doubledRecoHits.begin(), doubledRecoHits.end(), recoHit3D) != doubledRecoHits.end();
  }), foundStereoHits.end());

  for (const CDCRecoHit3D* hit : foundStereoHits) {
    if (hit->getWireHit().getAutomatonCell().hasTakenFlag())
      B2FATAL("Adding already found hit");
    track.push_back(*hit);
    hit->getWireHit().getAutomatonCell().setTakenFlag();
  }

  for (const CDCRecoHit3D* recoHit : hitsVector) {
    delete recoHit;
  }
}

void StereohitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track)
{
  std::vector<const CDCRecoHit3D*> hitsVector;
  fillHitsVector(hitsVector, track);

  m_hitQuadTree.seed(hitsVector);

  const std::vector<pair<Z0TanLambdaBox, vector<const CDCRecoHit3D*>>>& foundStereoHitsWithNode =
    m_hitQuadTree.findSingleBest(m_param_minimumNumberOfHits);

  m_hitQuadTree.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return;

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;

  std::vector<const CDCRecoHit3D*> doubledRecoHits;
  doubledRecoHits.reserve(foundStereoHits.size() / 2);

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    const CDCHit* currentHit = (*outerIterator)->getWireHit().getHit();
    for (auto innerIterator = foundStereoHits.begin(); innerIterator != outerIterator; ++innerIterator) {
      if (currentHit == (*innerIterator)->getWireHit().getHit()) {
        const double lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());
        const double lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());

        const double zSlopeMean = (node.getLowerTanLambda() + node.getUpperTanLambda()) / 2.0;

        if (fabs((lambda11 + lambda12) / 2 - zSlopeMean) < fabs((lambda21 + lambda22) / 2 - zSlopeMean)) {
          doubledRecoHits.push_back(*outerIterator);
        } else {
          doubledRecoHits.push_back(*innerIterator);
        }
      }
    }
  }

  addMaximumNodeToTrackAndDeleteHits(track, foundStereoHits, doubledRecoHits, hitsVector);
}

void StereohitsProcesser::reconstructSegment(const CDCRecoSegment2D& segment, std::vector<const CDCRecoSegment3D*>& recoSegments,
                                             const CDCTrajectory2D& trackTrajectory, const double /*maximumPerpS unused in the moment*/) const
{
  const double radius = trackTrajectory.getGlobalCircle().absRadius();
  const bool isCurler = trackTrajectory.isCurler();

  CDCRecoSegment3D newRecoSegment = CDCRecoSegment3D::reconstruct(segment, trackTrajectory);

  // Skip segments out of the CDC
  unsigned int numberOfHitsNotInCDC = 0;
  for (const CDCRecoHit3D& recoHit : newRecoSegment) {
    if (not recoHit.isInCellZBounds(1.1)) {
      numberOfHitsNotInCDC++;
    }
  }
  if (numberOfHitsNotInCDC > newRecoSegment.size() - 1) {
    return;
  }

  // Skip segments with hits on the wrong side (of not curlers)
  bool oneHitIsOnWrongSide = false;
  for (CDCRecoHit3D& recoHit : newRecoSegment) {
    const double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D < 0) {
      oneHitIsOnWrongSide = true;
      recoHit.setArcLength2D(currentArcLength2D + 2 * TMath::Pi() * radius);
    }
  }

  if (not isCurler and m_param_checkForB2BTracks and oneHitIsOnWrongSide) {
    return;
  }

  recoSegments.push_back(new CDCRecoSegment3D(newRecoSegment));
}

void StereohitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments)
{
  // Reconstruct the segments
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double maximumPerpS = track.back().getArcLength2D();

  std::vector<const CDCRecoSegment3D*> recoSegments;
  for (const CDCRecoSegment2D& segment : segments) {
    // Skip axial segments
    if (segment.getStereoType() == StereoType::c_Axial) {
      continue;
    }

    // Skip fully taken segments
    if (segment.isFullyTaken()) {
      continue;
    }

    if (segment.getTrajectory2D().getChargeSign() == trajectory2D.getChargeSign()) {
      reconstructSegment(segment, recoSegments, trajectory2D, maximumPerpS);
    } else {
      reconstructSegment(std::move(segment.reversed()), recoSegments, trajectory2D, maximumPerpS);
    }

  }

  // Do the tree finding
  m_segmentQuadTree.seed(recoSegments);
  const auto& foundStereoSegmentsWithNode = m_segmentQuadTree.findSingleBest(m_param_minimumNumberOfHits);
  m_segmentQuadTree.fell();

  if (foundStereoSegmentsWithNode.size() == 1) {
    // Add the hits of the segments to the track
    const auto& foundStereoSegmentWithNode = foundStereoSegmentsWithNode[0];
    const std::vector<const CDCRecoSegment3D*>& foundSegments = foundStereoSegmentWithNode.second;

    for (const CDCRecoSegment3D* segment : foundSegments) {
      for (const CDCRecoHit3D& recoHit : *segment) {
        if (recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
          B2WARNING("Adding already found hit");
          continue;
        }
        track.push_back(recoHit);
        recoHit.getWireHit().getAutomatonCell().setTakenFlag();
      }
    }
  }

  for (const CDCRecoSegment3D* recoSegment : recoSegments) {
    delete recoSegment;
  }
}
