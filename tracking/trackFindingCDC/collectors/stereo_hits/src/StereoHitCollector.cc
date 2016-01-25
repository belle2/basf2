#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitCollector.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitCollector::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  moduleParameters->addParameter(prefix + "quadTreeLevel", m_param_quadTreeLevel,
                                 "The number of levels for the quad tree search.",
                                 m_param_quadTreeLevel);

  moduleParameters->addParameter(prefix + "minimumNumberOfHits", m_param_minimumNumberOfHits,
                                 "The minimum number of hits in a quad tree bin to be called as result.",
                                 m_param_minimumNumberOfHits);

  moduleParameters->addParameter(prefix + "checkForB2BTracks", m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);

  moduleParameters->addParameter(prefix + "checkForDoubleHits", m_param_checkForDoubleHits,
                                 "Instead of deleting used hits, leave them where they are and check for double assignment later",
                                 m_param_checkForDoubleHits);
}

namespace {
  /** Returns a bool if the rlWire can be used as a stereo hit. */
  bool isValidStereoHit(const CDCRLTaggedWireHit& wireHit)
  {
    return not(wireHit.getStereoKind() == EStereoKind::c_Axial or wireHit.getWireHit().getAutomatonCell().hasTakenFlag());
    // TODO Check for number of layers in between
  }
}

void StereoHitCollector::reconstructHit(const CDCRLTaggedWireHit& rlWireHit, std::vector<CDCRecoHit3D>& hitsVector,
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
  hitsVector.emplace_back(rlWireHit, recoPos3D, perpS);
}


void StereoHitCollector::fillHitsVector(std::vector<CDCRecoHit3D>& hitsVector, const CDCTrack& track,
                                        const std::vector<CDCRLTaggedWireHit>& wireHits) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  hitsVector.reserve(wireHits.size());

  for (const CDCRLTaggedWireHit& wireHit : wireHits) {
    if (isValidStereoHit(wireHit)) {
      reconstructHit(wireHit, hitsVector, trajectory2D, isCurler, radius);
    }
  }
}

std::vector<std::pair<const CDCRLTaggedWireHit*, Weight>> StereoHitCollector::match(const CDCTrack& track,
                                                       const std::vector<CDCRLTaggedWireHit>& rlWireHits)
{
  // Reconstruct the hits to the track
  std::vector<CDCRecoHit3D> recoHits;
  fillHitsVector(recoHits, track, rlWireHits);

  // Do the tree finding
  m_hitQuadTree.seed(recoHits);
  const auto& foundStereoHitsWithNode = m_hitQuadTree.findSingleBest(m_param_minimumNumberOfHits);
  m_hitQuadTree.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return {};

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;

  for (const CDCRecoHit3D& recoHit : foundStereoHits) {
    recoHit.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  // Copy all usable hits (not the duplicates) into this list.
  std::vector<CDCRecoHit3D> hitsToUse;

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    bool isDoubled = false;

    const CDCHit* currentHitOuter = (*outerIterator)->getWireHit().getHit();

    if (outerIterator->getWireHit().getAutomatonCell().hasMaskedFlag()) {
      continue;
    }

    for (auto innerIterator = outerIterator; innerIterator != foundStereoHits.end(); ++innerIterator) {
      if (innerIterator->getWireHit().getAutomatonCell().hasMaskedFlag()) {
        continue;
      }

      const CDCHit* currentHitInner = (*innerIterator)->getWireHit().getHit();

      if (innerIterator != outerIterator and currentHitOuter == currentHitInner) {
        const double lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());
        const double lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());

        const double zSlopeMean = (node.getLowerTanLambda() + node.getUpperTanLambda()) / 2.0;

        if (fabs((lambda11 + lambda12) / 2 - zSlopeMean) < fabs((lambda21 + lambda22) / 2 - zSlopeMean)) {
          hitsToUse.push_back(*innerIterator);
          innerIterator->getWireHit().getAutomatonCell().setMaskedFlag();
          isDoubled = true;
          break;
        } else {
          hitsToUse.push_back(*outerIterator);
          outerIterator->getWireHit().getAutomatonCell().setMaskedFlag();
          isDoubled = true;
          break;
        }
      }
    }

    if (not isDoubled) {
      hitsToUse.push_back(*outerIterator);
    }
  }

  for (const CDCRecoHit3D& recoHit : foundStereoHits) {
    recoHit.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  for (const CDCRecoHit3D& recoHit : hitsToUse) {
    //track.push_back(recoHit);
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  std::vector<std::pair<const CDCRLTaggedWireHit*, Weight>> matches;
  const double defaultWeight = track.getStartTrajectory3D().getTrajectory2D().getAbsMom2D();
  for (const CDCRLTaggedWireHit& rlWireHit : rlWireHits) {
    auto foundInList = std::find_if(hitsToUse.begin(), hitsToUse.end(), [&rlWireHit](const CDCRecoHit3D & recoHit) {
      return recoHit.getWireHit().getHit() == rlWireHit.getHit() and recoHit.getRLInfo() == rlWireHit.getRLInfo();
    });

    if (foundInList != hitsToUse.end()) {
      matches.emplace_back(&rlWireHit, defaultWeight);
    }
  }
  return matches;
}

void StereoHitCollector::add(CDCTrack& track, const std::vector<const CDCRLTaggedWireHit*>& matchedHits)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  for (const CDCRLTaggedWireHit* rlWireHit : matchedHits) {
    Vector3D recoPos3D = rlWireHit->reconstruct3D(trajectory2D);
    double arcLength2D = trajectory2D.calcArcLength2D(recoPos3D.xy());
    if (isCurler and arcLength2D < 0) {
      arcLength2D += 2 * TMath::Pi() * radius;
    }

    track.emplace_back(*rlWireHit, recoPos3D, arcLength2D);
  }
}
