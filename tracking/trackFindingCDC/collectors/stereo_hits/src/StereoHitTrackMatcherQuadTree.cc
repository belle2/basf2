#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <utility>
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherQuadTree.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitTrackMatcherQuadTree::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  QuadTreeBasedMatcher<HitZ0TanLambdaLegendre>::exposeParameters(moduleParameters, prefix);
  m_filterFactory.exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefix + "checkForB2BTracks", m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);

}

Weight StereoHitTrackMatcherQuadTree::getWeight(const CDCRecoHit3D& recoHit, const Z0TanLambdaBox& node,
                                                const CDCTrack& track) const
{
  return m_stereoHitFilter->operator()({&recoHit, &track});
}

std::vector<WithWeight<const CDCRLTaggedWireHit*>> StereoHitTrackMatcherQuadTree::match(const CDCTrack& track,
                                                const std::vector<CDCRLTaggedWireHit>& rlWireHits)
{
  if (m_stereoHitFilter->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // Reconstruct the hits to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  typedef std::pair<CDCRecoHit3D, const CDCRLTaggedWireHit*> CDCRecoHitWithRLPointer;
  std::vector<CDCRecoHitWithRLPointer> recoHits;
  recoHits.reserve(rlWireHits.size());

  /*
   * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
   * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
   */
  for (const CDCRLTaggedWireHit& rlWireHit : rlWireHits) {
    if (rlWireHit.getStereoKind() != EStereoKind::c_Axial and not rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      // Skip hits out of CDC
      if (not wire.isInCellZBounds(recoPos3D)) {
        continue;
      }

      // If the track is a curler, shift all perpS values to positive ones. Else do not use this hit if m_param_checkForB2BTracks is enabled.
      double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
      if (perpS < 0) {
        if (isCurler) {
          perpS += 2 * TMath::Pi() * radius;
        } else if (m_param_checkForB2BTracks) {
          continue;
        }
      }
      recoHits.emplace_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS), &rlWireHit);
    }
  }

  // Do the tree finding
  m_quadTreeInstance.seed(recoHits);
  const auto& foundStereoHitsWithNode = m_quadTreeInstance.findSingleBest(m_param_minimumNumberOfHits);
  m_quadTreeInstance.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return {};

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;

  for (const auto& recoHit : foundStereoHits) {
    recoHit.first.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  // Copy all usable hits (not the duplicates) into this list.
  std::vector<WithWeight<const CDCRLTaggedWireHit*>> matches;

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    bool isDoubled = false;

    const CDCRecoHit3D& currentRecoHitOuter = outerIterator->first;
    const CDCRLTaggedWireHit* currentRLWireHitOuter = outerIterator->second;
    const CDCWireHit& currentWireHitOuter = currentRLWireHitOuter->getWireHit();
    const CDCHit* currentHitOuter = currentWireHitOuter.getHit();

    if (currentWireHitOuter.getAutomatonCell().hasMaskedFlag()) {
      continue;
    }

    for (auto innerIterator = outerIterator; innerIterator != foundStereoHits.end(); ++innerIterator) {
      const CDCRecoHit3D& currentRecoHitInner = innerIterator->first;
      const CDCRLTaggedWireHit* currentRLWireHitInner = innerIterator->second;
      const CDCWireHit& currentWireHitInner = currentRLWireHitInner->getWireHit();
      const CDCHit* currentHitInner = currentWireHitInner.getHit();

      if (currentWireHitInner.getAutomatonCell().hasMaskedFlag()) {
        continue;
      }

      if (innerIterator != outerIterator and currentHitOuter == currentHitInner) {
        const double lambda11 = 1 / currentRecoHitInner.calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda12 = 1 / currentRecoHitInner.calculateZSlopeWithZ0(node.getUpperZ0());
        const double lambda21 = 1 / currentRecoHitOuter.calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda22 = 1 / currentRecoHitOuter.calculateZSlopeWithZ0(node.getUpperZ0());

        const double zSlopeMean = (node.getLowerTanLambda() + node.getUpperTanLambda()) / 2.0;

        if (fabs((lambda11 + lambda12) / 2 - zSlopeMean) < fabs((lambda21 + lambda22) / 2 - zSlopeMean)) {
          const Weight weight = getWeight(currentRecoHitInner, node, track);
          if (not std::isnan(weight)) {
            matches.emplace_back(currentRLWireHitInner, weight);
          }
        } else {
          const Weight weight = getWeight(currentRecoHitOuter, node, track);
          if (not std::isnan(weight)) {
            matches.emplace_back(currentRLWireHitOuter, weight);
          }
        }

        // currentWireHitInner = currentWireHitOuter, so it makes no difference here
        currentWireHitInner.getAutomatonCell().setMaskedFlag();
        isDoubled = true;
        break;
      }
    }

    if (not isDoubled) {
      const Weight weight = getWeight(currentRecoHitOuter, node, track);
      if (not std::isnan(weight)) {
        matches.emplace_back(currentRLWireHitOuter, weight);
      }
    }
  }

  for (const auto& recoHit : foundStereoHits) {
    recoHit.first.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  return matches;
}
