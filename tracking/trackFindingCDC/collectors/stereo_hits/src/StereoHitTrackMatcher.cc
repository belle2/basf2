#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcher.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /** Returns a bool if the rlWire can be used as a stereo hit. */
  bool isValidStereoHit(const CDCRLTaggedWireHit& wireHit)
  {
    return not(wireHit.getStereoKind() == EStereoKind::c_Axial or wireHit.getWireHit().getAutomatonCell().hasTakenFlag());
    // TODO Check for number of layers in between
  }
}

void StereoHitTrackMatcher::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{

  QuadTreeBasedMatcher<HitZ0TanLambdaLegendre>::exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefix + "checkForB2BTracks", m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);
}

void StereoHitTrackMatcher::reconstructHit(const CDCRLTaggedWireHit& rlWireHit, std::vector<CDCRecoHitWithRLPointer>& hitsVector,
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
  hitsVector.emplace_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS), &rlWireHit);
}

std::vector<WithWeight<const CDCRLTaggedWireHit*>> StereoHitTrackMatcher::match(const CDCTrack& track,
                                                const std::vector<CDCRLTaggedWireHit>& rlWireHits)
{
  // Reconstruct the hits to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  std::vector<CDCRecoHitWithRLPointer> recoHits;
  recoHits.reserve(rlWireHits.size());

  for (const CDCRLTaggedWireHit& wireHit : rlWireHits) {
    if (isValidStereoHit(wireHit)) {
      reconstructHit(wireHit, recoHits, trajectory2D, isCurler, radius);
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
  const Weight weight = track.size();

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
          matches.emplace_back(currentRLWireHitInner, weight);
        } else {
          matches.emplace_back(currentRLWireHitOuter, weight);
        }

        // currentWireHitInner = currentWireHitOuter, so it makes no difference here
        currentWireHitInner.getAutomatonCell().setMaskedFlag();
        isDoubled = true;
        break;
      }
    }

    if (not isDoubled) {
      matches.emplace_back(currentRLWireHitOuter, weight);
    }
  }

  for (const auto& recoHit : foundStereoHits) {
    recoHit.first.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  return matches;
}
