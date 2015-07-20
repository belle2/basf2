#include <tracking/trackFindingCDC/legendre/stereohits/CDCLegendreStereohitsProcesser.h>

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

// New Quad Tree
#include <tracking/trackFindingCDC/hough/z0_zslope/HitZ0ZSlopeLegendre.h>

#include <TFile.h>
#include <TH2F.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool StereohitsProcesser::rlWireHitMatchesTrack(const CDCRLWireHit& rlWireHit, const CDCTrajectory2D& trajectory2D) const
{
  if (rlWireHit.getStereoType() == AXIAL or rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag())
    return false;

  const Vector2D& center = trajectory2D.getGlobalCircle().center();
  double trackPhi = center.phi();
  double hitPhi = rlWireHit.getRefPos2D().phi();


  double phi_diff = trackPhi - hitPhi;
  if (not std::isnan(phi_diff)) {
    phi_diff = TVector2::Phi_0_2pi(phi_diff);
  }

  int charge = 1;
  if (phi_diff <= TMath::Pi())
    charge = -1;

  if (trajectory2D.getChargeSign() != charge)
    return false;

  return true;
}

void StereohitsProcesser::fillHitsVector(std::vector<HitType*>& hitsVector, const CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  hitsVector.reserve(wireHitTopology.getRLWireHits().size());

  for (const CDCRLWireHit& rlWireHit : wireHitTopology.getRLWireHits()) {
    if (rlWireHitMatchesTrack(rlWireHit, trajectory2D)) {
      const CDCWire& wire = rlWireHit.getWire();
      const double forwardZ = wire.getSkewLine().forwardZ();
      const double backwardZ = wire.getSkewLine().backwardZ();

      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      if (backwardZ < recoPos3D.z() and recoPos3D.z() < forwardZ) {
        FloatType perpS = trajectory2D.calcPerpS(recoPos3D.xy());
        hitsVector.push_back(new CDCRecoHit3D(&(rlWireHit), recoPos3D, perpS));
      }
    }
  }
}

void StereohitsProcesser::addMaximumNodeToTrackAndDeleteHits(CDCTrack& track, std::vector<HitType*>& foundStereoHits,
    const std::vector<HitType*> doubledRecoHits, const std::vector<HitType*> hitsVector) const
{
  foundStereoHits.erase(std::remove_if(foundStereoHits.begin(),
  foundStereoHits.end(), [&doubledRecoHits](HitType * recoHit3D) -> bool {
    return std::find(doubledRecoHits.begin(), doubledRecoHits.end(), recoHit3D) != doubledRecoHits.end();
  }), foundStereoHits.end());

  for (HitType* hit : foundStereoHits) {
    if (hit->getWireHit().getAutomatonCell().hasTakenFlag()) B2FATAL("Adding already found hit")
      track.push_back(*hit);
    hit->getWireHit().getAutomatonCell().setTakenFlag();
  }

  for (HitType* recoHit : hitsVector) {
    delete recoHit;
  }
}

void StereohitsProcesser::makeHistogramming(CDCTrack& track, unsigned int m_param_level, unsigned int m_param_minimumHits)
{
  typedef TrackFindingCDC::StereoHitQuadTreeProcessor Processor;

  // Ranges: slope, z0
  Processor::ChildRanges ranges(Processor::rangeX(tan(-75.* TMath::Pi() / 180.), tan(75.* TMath::Pi() / 180.)),
                                Processor::rangeY(-20, 20));


  typedef std::pair<Processor::QuadTree*, Processor::ReturnList> Result;
  std::vector<Result> possibleStereoSegments;

  Processor::ReturnList hitsVector;
  fillHitsVector(hitsVector, track);

  Processor::CandidateProcessorLambda lmdCandidateProcessing = [&](const Processor::ReturnList & items,
  Processor::QuadTree * node) -> void {
    possibleStereoSegments.push_back(std::make_pair(node, std::move(items)));
    B2DEBUG(100, "Lambda: " << node->getXMean() << "; Z0: " << node->getYMean() << "; nhits: " << items.size());
  };

  unsigned int level = m_param_level;
  Processor qtProcessor(level, ranges, m_param_debugOutput);
  qtProcessor.provideItemsSet(hitsVector);
  qtProcessor.fillGivenTree(lmdCandidateProcessing, m_param_minimumHits);

  /* DEBUG */
  if (m_param_debugOutput) {
    // Debug output
    const auto& debugMap = qtProcessor.getDebugInformation();

    TFile file("quadtree_content.root", "RECREATE");
    TH2F hist("hist", "QuadTreeContent", std::pow(2, level), ranges.first.first, ranges.first.second, std::pow(2, level),
              ranges.second.first, ranges.second.second);

    for (const auto& debug : debugMap) {
      const auto& positionInformation = debug.first;
      const auto& quadItemsVector = debug.second;
      hist.Fill(positionInformation.first, positionInformation.second, quadItemsVector.size());
    }

    hist.Write();
    file.Clone();
  }
  /* DEBUG */

  if (possibleStereoSegments.size() == 0)
    return;

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
        FloatType lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMin());
        FloatType lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node->getYMax());
        FloatType lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMin());
        FloatType lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node->getYMax());

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


void StereohitsProcesser::makeHistogrammingWithNewQuadTree(CDCTrack& track, unsigned int m_param_level,
                                                           unsigned int m_param_minimumHits)
{
  // Prepare the hough algorithm
  const size_t maxLevel = m_param_level;
  const size_t z0Divisions = 2;
  const size_t inverseZSlopeDivisions = 2;

  using HitZ0ZSlopeQuadLegendre = HitZ0ZSlopeLegendre<const HitType*, z0Divisions, inverseZSlopeDivisions>;
  HitZ0ZSlopeQuadLegendre hitZ0ZSlopeQuadLegendre(maxLevel);
  hitZ0ZSlopeQuadLegendre.initialize();

  std::vector<HitType*> hitsVector;
  fillHitsVector(hitsVector, track);

  typedef pair<Z0ZSlopeBox, vector<HitType*>> Result;
  vector<Result> possibleStereoSegments;
  hitZ0ZSlopeQuadLegendre.seed(hitsVector);
  possibleStereoSegments = hitZ0ZSlopeQuadLegendre.find(m_param_minimumHits);

  hitZ0ZSlopeQuadLegendre.fell();
  hitZ0ZSlopeQuadLegendre.raze();

  if (possibleStereoSegments.size() == 0)
    return;

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
        const FloatType& lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const FloatType& lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());
        const FloatType& lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const FloatType& lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());

        const FloatType& zSlopeMean = (node.getLowerZSlope() + node.getUpperZSlope()) / 2.0;

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
