#include <tracking/trackFindingCDC/legendre/stereohits/StereohitsProcesser.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/legendre/quadtree/StereoHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>


#include <TFile.h>
#include <TH2F.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool StereohitsProcesser::isValidHitTrajectoryMatch(const CDCRLWireHit& rlWireHit, const CDCTrajectory2D& trajectory2D) const
{
  if (rlWireHit.getStereoType() == StereoType::c_Axial or rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag())
    return false;


  // Check for number of layers in between

  return true;
}

void StereohitsProcesser::fillHitsVector(std::vector<HitType*>& hitsVector, const CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const double radius = trajectory2D.getGlobalCircle().radius();
  const bool isCurler = trajectory2D.getOuterExit().hasNAN();

  const auto& rlWireHits = wireHitTopology.getRLWireHits();
  hitsVector.reserve(rlWireHits.size());

  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (isValidHitTrajectoryMatch(rlWireHit, trajectory2D)) {

      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      if (wire.isInCellZBounds(recoPos3D)) {
        double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
        if (perpS < 0) {
          if (isCurler) {
            perpS += 2 * TMath::Pi() * radius;
          } else if (m_checkForB2BTracks) {
            continue;
          }
        }
        CDCRecoHit3D* newRecoHit = new CDCRecoHit3D(&(rlWireHit), recoPos3D, perpS);
        hitsVector.push_back(newRecoHit);
      }
    }
  }
}

void StereohitsProcesser::addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<HitType*>& foundStereoHits,
    const std::vector<HitType*>& doubledRecoHits, const std::vector<HitType*>& hitsVector) const
{
  B2INFO("Number of doubled hits: " << doubledRecoHits.size());

  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
  foundStereoHits.end(), [&doubledRecoHits](HitType * recoHit3D) -> bool {
    return std::find(doubledRecoHits.begin(), doubledRecoHits.end(), recoHit3D) != doubledRecoHits.end();
  }), foundStereoHits.end());

  for (HitType* hit : foundStereoHits) {
    if (hit->getWireHit().getAutomatonCell().hasTakenFlag())
      B2FATAL("Adding already found hit");
    track.push_back(*hit);
    hit->getWireHit().getAutomatonCell().setTakenFlag();
  }

  for (HitType* recoHit : hitsVector) {
    delete recoHit;
  }
}

void StereohitsProcesser::makeHistogramming(CDCTrack& track, unsigned int m_param_minimumHits)
{
  typedef std::pair<StereoHitQuadTreeProcessor::QuadTree*, StereoHitQuadTreeProcessor::ReturnList> Result;
  std::vector<Result> possibleStereoSegments;

  StereoHitQuadTreeProcessor::ReturnList hitsVector;
  fillHitsVector(hitsVector, track);

  StereoHitQuadTreeProcessor::CandidateProcessorLambda lmdCandidateProcessing = [&](const StereoHitQuadTreeProcessor::ReturnList &
      items,
  StereoHitQuadTreeProcessor::QuadTree * node) -> void {
    possibleStereoSegments.push_back(std::make_pair(node, std::move(items)));
    B2DEBUG(100, "Lambda: " << node->getXMean() << "; Z0: " << node->getYMean() << "; nhits: " << items.size());
  };

  StereoHitQuadTreeProcessor::ChildRanges childRanges = StereoHitQuadTreeProcessor::ChildRanges(StereoHitQuadTreeProcessor::rangeX(
                                                          tan(-75.* TMath::Pi() / 180.), tan(75.* TMath::Pi() / 180.)), StereoHitQuadTreeProcessor::rangeY(-100, 100));
  StereoHitQuadTreeProcessor oldQuadTree(m_level, childRanges, m_param_debugOutput);
  oldQuadTree.provideItemsSet(hitsVector);
  oldQuadTree.fillGivenTree(lmdCandidateProcessing, m_param_minimumHits);

  /* DEBUG */
  if (m_param_debugOutput) {
    const auto& debugMap = oldQuadTree.getDebugInformation();

    TFile file("quadtree_content.root", "RECREATE");
    TH2F hist("hist", "QuadTreeContent", std::pow(2, m_level), childRanges.first.first, childRanges.first.second,
              std::pow(2, m_level), childRanges.second.first, childRanges.second.second);

    for (const auto& debug : debugMap) {
      const auto& positionInformation = debug.first;
      const auto& quadItemsVector = debug.second;
      hist.Fill(positionInformation.first, positionInformation.second, quadItemsVector.size());
    }

    hist.Write();
    file.Close();
  }

  if (possibleStereoSegments.size() == 0) {
    B2WARNING("Found no stereo hits!");
    return;
  }

  auto maxList = std::max_element(possibleStereoSegments.begin(), possibleStereoSegments.end(), [](const Result & a,
  const Result & b) {
    return a.second.size() < b.second.size();
  });

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto& foundStereoHits = maxList->second;
  auto node = maxList->first;

  std::vector<HitType*> doubledRecoHits;
  doubledRecoHits.reserve(foundStereoHits.size() / 2);

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    const CDCHit* currentHit = (*outerIterator)->getWireHit().getHit();
    for (auto innerIterator = foundStereoHits.begin(); innerIterator != outerIterator; ++innerIterator) {
      if (currentHit == (*innerIterator)->getWireHit().getHit()) {
        double lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMin());
        double lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMax());
        double lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMin());
        double lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMax());

        if (fabs((lambda11 + lambda12) / 2 - node->getXMean()) < fabs((lambda21 + lambda22) / 2 - node->getXMean())) {
          doubledRecoHits.push_back(*outerIterator);
        } else {
          doubledRecoHits.push_back(*innerIterator);
        }
      }

    }
  }

  addMaximumNodeToTrackAndDeleteHits(track, foundStereoHits, doubledRecoHits, hitsVector);
}

void StereohitsProcesser::makeHistogrammingWithNewQuadTree(CDCTrack& track, unsigned int minimumNumberOfHits)
{
  std::vector<HitType*> hitsVector;
  fillHitsVector(hitsVector, track);

  m_newQuadTree.seed(hitsVector);

  const std::vector<pair<Z0TanLambdaBox, vector<HitType*>>>& foundStereoHitsWithNode =
    m_newQuadTree.findSingleBest(minimumNumberOfHits);

  m_newQuadTree.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return;

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;

  std::vector<HitType*> doubledRecoHits;
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

void StereohitsProcesser::makeHistogrammingWithSegments(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments,
                                                        unsigned int minimumNumberOfHits)
{
  // Reconstruct the segments
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().radius();
  const bool isCurler = trajectory2D.getOuterExit().hasNAN();

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

    CDCRecoSegment3D newRecoSegment = CDCRecoSegment3D::reconstruct(segment, trajectory2D);

    // Skip segments out of the CDC
    unsigned int numberOfHitsNotInCDC = 0;
    for (const CDCRecoHit3D& recoHit : newRecoSegment) {
      if (not recoHit.isInCellZBounds(1.5)) {
        numberOfHitsNotInCDC++;
      }
    }
    if (numberOfHitsNotInCDC > 0.7 * newRecoSegment.size()) {
      continue;
    }

    // Skip segments with hits on the wrong side (of not curlers)
    bool oneHitIsOnWrongSide = false;
    bool oneHitIsTooFarAway = false;
    for (CDCRecoHit3D& recoHit : newRecoSegment) {
      const double currentArcLength2D = recoHit.getArcLength2D();
      if (currentArcLength2D < 0) {
        oneHitIsOnWrongSide = true;
        recoHit.setArcLength2D(currentArcLength2D + 2 * TMath::Pi() * radius);
      }

      if (recoHit.getArcLength2D() - maximumPerpS > 30) {
        oneHitIsTooFarAway = true;
      }
    }

    if (oneHitIsTooFarAway) {
      continue;
    }

    if (not isCurler and m_checkForB2BTracks and oneHitIsOnWrongSide) {
      continue;
    }

    recoSegments.push_back(new CDCRecoSegment3D(newRecoSegment));
  }

  // Do the tree finding
  m_segmentQuadTree.seed(recoSegments);
  const auto& foundStereoSegmentsWithNode = m_segmentQuadTree.findSingleBest(minimumNumberOfHits);
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
