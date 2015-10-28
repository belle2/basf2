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


/** Returns a bool if the rlWire can be used as a stereo hit. */
bool isValidStereoHit(const CDCRLWireHit& rlWireHit)
{
  return not(rlWireHit.getStereoType() == StereoType::c_Axial or rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag());
  // TODO Check for number of layers in between
}

/** Returns a bool if the segment can be used as a stereo hit. */
bool isValidStereoSegment(const CDCRecoSegment2D& segment)
{
  // Skip axial segments
  return not(segment.getStereoType() == StereoType::c_Axial or segment.isFullyTaken());
}

void StereohitsProcesser::reconstructHit(const CDCRLWireHit& rlWireHit, std::vector<const CDCRecoHit3D*>& hitsVector,
                                         const CDCTrajectory2D& trackTrajectory, const bool isCurler, const double radius) const
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trackTrajectory);
  const CDCWire& wire = rlWireHit.getWire();
  // Skip hits out of CDC
  if (not wire.isInCellZBounds(recoPos3D)) {
    return;
  }

  // If the track is a curler, shift all perpS values to positive ones. Else do not use this hit if m_param_checkForB2BTracks is enabled.
  double perpS = trackTrajectory.calcArcLength2D(recoPos3D.xy());
  if (perpS < 0) {
    if (isCurler) {
      perpS += 2 * TMath::Pi() * radius;
    } else if (m_param_checkForB2BTracks) {
      return;
    }
  }
  CDCRecoHit3D* newRecoHit = new CDCRecoHit3D(rlWireHit, recoPos3D, perpS);
  hitsVector.push_back(newRecoHit);
}

void StereohitsProcesser::reconstructSegment(const CDCRecoSegment2D& segment,
                                             std::vector<const CDCRecoSegment3D*>& recoSegmentVector,
                                             const CDCTrajectory2D& trackTrajectory) const
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

  // Do not add this segment if all of the hits are out of the CDC
  if (numberOfHitsNotInCDC > newRecoSegment.size() - 1) {
    return;
  }

  // Skip segments with hits on the wrong side (if not curlers)
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

  CDCRecoSegment3D* newRecoSegmentPtr = new CDCRecoSegment3D(newRecoSegment);
  recoSegmentVector.push_back(newRecoSegmentPtr);
}


void StereohitsProcesser::fillHitsVector(std::vector<const CDCRecoHit3D*>& hitsVector, const CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.getOuterExit().hasNAN();

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const auto& rlWireHits = wireHitTopology.getRLWireHits();
  hitsVector.reserve(rlWireHits.size());

  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (isValidStereoHit(rlWireHit)) {
      reconstructHit(rlWireHit, hitsVector, trajectory2D, isCurler, radius);
    }
  }
}

void StereohitsProcesser::fillSegmentsVector(std::vector<const CDCRecoSegment3D*> recoSegmentsVector,
                                             const std::vector<CDCRecoSegment2D>& segments, CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  recoSegmentsVector.reserve(segments.size());

  for (const CDCRecoSegment2D& segment : segments) {
    if (isValidStereoSegment(segment)) {
      // Reverse the segments if needed
      if (segment.getTrajectory2D().getChargeSign() == trajectory2D.getChargeSign()) {
        reconstructSegment(segment, recoSegmentsVector, trajectory2D);
      } else {
        reconstructSegment(std::move(segment.reversed()), recoSegmentsVector, trajectory2D);
      }
    }
  }
}

void StereohitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track)
{
  // Reconstruct the hits to the track
  std::vector<const CDCRecoHit3D*> recoHits;
  fillHitsVector(recoHits, track);

  // Do the tree finding
  m_hitQuadTree.seed(recoHits);
  const auto& foundStereoHitsWithNode = m_hitQuadTree.findSingleBest(m_param_minimumNumberOfHits);
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

  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
  foundStereoHits.end(), [&doubledRecoHits](const CDCRecoHit3D * recoHit3D) -> bool {
    return std::find(doubledRecoHits.begin(), doubledRecoHits.end(), recoHit3D) != doubledRecoHits.end();
  }), foundStereoHits.end());

  // Add the hits to the track
  for (const CDCRecoHit3D* recoHit : foundStereoHits) {
    B2ASSERT("Found hit should not have taken flag!", not recoHit->getWireHit().getAutomatonCell().hasTakenFlag());
    track.push_back(*recoHit);
    recoHit->getWireHit().getAutomatonCell().setTakenFlag();
  }

  for (const CDCRecoHit3D* recoHit : recoHits) {
    delete recoHit;
  }
}


void StereohitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments)
{
  // Reconstruct the segments
  std::vector<const CDCRecoSegment3D*> recoSegments;
  fillSegmentsVector(recoSegments, segments, track);

  // Do the tree finding
  m_segmentQuadTree.seed(recoSegments);
  const auto& foundStereoSegmentsWithNode = m_segmentQuadTree.findSingleBest(m_param_minimumNumberOfHits);
  m_segmentQuadTree.fell();

  if (foundStereoSegmentsWithNode.size() != 1) {
    return;
  }

  // Add the hits of the segments to the track
  const auto& foundStereoSegmentWithNode = foundStereoSegmentsWithNode[0];
  const std::vector<const CDCRecoSegment3D*>& foundSegments = foundStereoSegmentWithNode.second;

  for (const CDCRecoSegment3D* segment : foundSegments) {
    for (const CDCRecoHit3D& recoHit : *segment) {
      B2ASSERT("Found hit should not have taken flag!", not recoHit.getWireHit().getAutomatonCell().hasTakenFlag());
      track.push_back(recoHit);
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }

  // Delete the list of found segments and free memory
  for (const CDCRecoSegment3D* recoSegment : recoSegments) {
    delete recoSegment;
  }
}

void StereohitsProcesser::postprocessTrack(CDCTrack& track) const
{
  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

  track.shiftToPositiveArcLengths2D();
  track.sortByArcLength2D();

  const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
  CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
  track.setStartTrajectory3D(newStartTrajectory);
}
