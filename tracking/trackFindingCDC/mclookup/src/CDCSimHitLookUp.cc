/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCSimHitLookUp& CDCSimHitLookUp::getInstance()
{
  return CDCMCManager::getSimHitLookUp();
}

void CDCSimHitLookUp::clear()
{
  m_ptrMCMap = nullptr;

  m_primarySimHits.clear();
  m_rightLeftInfos.clear();
}

void CDCSimHitLookUp::fill(const CDCMCMap* ptrMCMap)
{
  B2DEBUG(100, "In CDCSimHitLookUp::fill()");
  clear();
  m_ptrMCMap = ptrMCMap;

  fillPrimarySimHits();
  fillRLInfo();

  B2DEBUG(100, "m_primarySimHits.size(): " << m_primarySimHits.size());
  B2DEBUG(100, "m_rightLeftInfos.size(): " << m_rightLeftInfos.size());
}

void CDCSimHitLookUp::fillPrimarySimHits()
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot setup primary sim hit map");
    return;
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;
  int nMissingPrimarySimHits = 0;
  for (const auto& relation : mcMap.getSimHitsByHit()) {

    const CDCHit* ptrHit = std::get<const CDCHit* const>(relation);
    const CDCSimHit* ptrSimHit = std::get<const CDCSimHit*>(relation);

    if (not ptrSimHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCSimHitLookUp::fill()");
      continue;
    }

    if (mcMap.isReassignedSecondary(ptrSimHit)) {
      MayBePtr<const CDCSimHit> primarySimHit = getClosestPrimarySimHit(ptrSimHit);
      if (not primarySimHit) {
        ++nMissingPrimarySimHits;
      }
      m_primarySimHits[ptrHit] = primarySimHit;
    }
  }
  if (nMissingPrimarySimHits != 0) {
    B2WARNING("NO primary hit found for " << nMissingPrimarySimHits << " reassigned secondaries");
  }
}

MayBePtr<const CDCSimHit> CDCSimHitLookUp::getClosestPrimarySimHit(const CDCSimHit* ptrSimHit) const
{
  if (not ptrSimHit) {
    return nullptr;
  }
  const CDCSimHit& simHit = *ptrSimHit;

  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot find primary sim hit");
    return nullptr;
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;

  // Check if the CDCSimHit was reassigned from a secondary particle to its primary particle.
  if (not mcMap.isReassignedSecondary(ptrSimHit)) {
    return ptrSimHit;
  } else {

    // Try to find the hit on the same wire from the primary particle.
    const MCParticle* ptrMCParticle = mcMap.getMCParticle(ptrSimHit);
    if (not ptrMCParticle) {
      return nullptr;
    }

    WireID wireID = simHit.getWireID();
    std::vector<const CDCSimHit*> primarySimHitsOnSameOrNeighborWire;
    const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

    for (const auto& simHitByMCParticleRelation : mcMap.getSimHits(ptrMCParticle)) {

      const CDCSimHit* ptrPrimarySimHit = std::get<const CDCSimHit*>(simHitByMCParticleRelation);
      if (mcMap.isReassignedSecondary(ptrPrimarySimHit) or not ptrPrimarySimHit) continue;

      const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

      if (wireTopology.arePrimaryNeighbors(primarySimHit.getWireID(), wireID) or
          primarySimHit.getWireID() == wireID) {

        // Found a hit on the same wire from the primary particle.
        primarySimHitsOnSameOrNeighborWire.push_back(ptrPrimarySimHit);
      }
    }

    // Now from the neighboring primary CDCSimHits pick to one with the smallest distance to the
    // secondary CDCSimHit.
    auto compareDistanceBetweenSimHits =
      [&simHit](const CDCSimHit * primarySimHit,
    const CDCSimHit * otherPrimarySimHit) -> bool {
      Vector3D primaryHitPos(primarySimHit->getPosTrack());
      Vector3D otherPrimaryHitPos(otherPrimarySimHit->getPosTrack());
      Vector3D secondaryHitPos(simHit.getPosTrack());
      return primaryHitPos.distance(secondaryHitPos) < otherPrimaryHitPos.distance(secondaryHitPos);
    };

    auto itClosestPrimarySimHit = std::min_element(primarySimHitsOnSameOrNeighborWire.begin(),
                                                   primarySimHitsOnSameOrNeighborWire.end(),
                                                   compareDistanceBetweenSimHits);

    if (itClosestPrimarySimHit != primarySimHitsOnSameOrNeighborWire.end()) {
      // Found primary simulated hit for secondary hit.
      return *itClosestPrimarySimHit;
    } else {
      return nullptr;
    }
  }
}

const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCHit* ptrHit) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set in look up of closest primary sim hit.");
    return nullptr;
  }
  const CDCMCMap& mcMap = *m_ptrMCMap;

  if (mcMap.isReassignedSecondary(ptrHit)) {
    auto itFoundPrimarySimHit = m_primarySimHits.find(ptrHit);
    if (itFoundPrimarySimHit != m_primarySimHits.end()) {
      const CDCSimHit* simHit = itFoundPrimarySimHit->second;
      if (simHit) return simHit;
    }
  }
  // Return the normal (potentially secondary) CDCSimHit of no primary is available
  return mcMap.getSimHit(ptrHit);
}

Vector3D CDCSimHitLookUp::getDirectionOfFlight(const CDCHit* ptrHit)
{
  if (not ptrHit) return Vector3D();

  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot find direction of flight");
    return Vector3D();
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;

  const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);

  if (not ptrSimHit) return Vector3D();

  const CDCSimHit* ptrPrimarySimHit =
    mcMap.isReassignedSecondary(ptrHit) ? getClosestPrimarySimHit(ptrHit) : ptrSimHit;

  if (not ptrPrimarySimHit) {
    // if no primary simhit is close to the secondary hit we can only take the secondary
    ptrPrimarySimHit = ptrSimHit;

    // or invent something better at some point...
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  // Take the momentum of the primary hit
  Vector3D directionOfFlight{primarySimHit.getMomentum()};
  return directionOfFlight;
}

void CDCSimHitLookUp::fillRLInfo()
{

  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot setup right left passage information map");
    return;
  }
  const CDCMCMap& mcMap = *m_ptrMCMap;

  for (const auto& relation : mcMap.getSimHitsByHit()) {

    const CDCHit* ptrHit = std::get<const CDCHit* const>(relation);
    const CDCSimHit* ptrSimHit = std::get<const CDCSimHit*>(relation);

    if (not ptrSimHit) continue;
    const CDCSimHit& simHit = *ptrSimHit;

    Vector3D directionOfFlight = getDirectionOfFlight(ptrHit);
    if (directionOfFlight.isNull()) continue;

    // find out if the wire is right or left of the track ( view in flight direction )
    Vector3D trackPosToWire{simHit.getPosWire() - simHit.getPosTrack()};
    ERightLeft rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());
    m_rightLeftInfos[ptrHit] = rlInfo;
  }
}

ERightLeft CDCSimHitLookUp::getRLInfo(const CDCHit* ptrHit) const
{
  auto itFoundHit = m_rightLeftInfos.find(ptrHit);
  return itFoundHit == m_rightLeftInfos.end() ? ERightLeft::c_Invalid : itFoundHit->second;
}

Vector3D CDCSimHitLookUp::getRecoPos3D(const CDCHit* ptrHit) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot find reconstructed position");
    return Vector3D();
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;
  const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);

  if (not ptrSimHit) {
    B2WARNING("No CDCSimHit related to CDCHit");
    return Vector3D();
  }

  const CDCSimHit& simHit = *ptrSimHit;
  return Vector3D{simHit.getPosTrack()};
}

double CDCSimHitLookUp::getDriftLength(const CDCHit* ptrHit) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot find reconstructed position");
    return NAN;
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;
  const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);

  if (not ptrSimHit) {
    B2WARNING("No CDCSimHit related to CDCHit");
    return NAN;
  }

  const CDCSimHit& simHit = *ptrSimHit;
  return simHit.getDriftLength();
}

Vector3D CDCSimHitLookUp::getClosestPrimaryRecoPos3D(const CDCHit* ptrHit) const
{
  const CDCSimHit* ptrPrimarySimHit = getClosestPrimarySimHit(ptrHit);
  if (ptrPrimarySimHit) {
    const CDCSimHit& primarySimHit = *ptrPrimarySimHit;
    return Vector3D{primarySimHit.getPosTrack()};
  } else {
    return getRecoPos3D(ptrHit);
  }
}
double CDCSimHitLookUp::getClosestPrimaryDriftLength(const CDCHit* ptrHit) const
{
  const CDCSimHit* ptrPrimarySimHit = getClosestPrimarySimHit(ptrHit);
  if (ptrPrimarySimHit) {
    const CDCSimHit& primarySimHit = *ptrPrimarySimHit;
    return primarySimHit.getDriftLength();
  } else {
    return getDriftLength(ptrHit);
  }
}

const CDCWireHit* CDCSimHitLookUp::getWireHit(const CDCHit* ptrHit,
                                              const std::vector<CDCWireHit>& wireHits) const
{
  if (not ptrHit) return nullptr;
  ConstVectorRange<CDCWireHit> wireHit{std::equal_range(wireHits.begin(), wireHits.end(), *ptrHit)};

  if (wireHit.empty()) {
    return nullptr;
  } else {
    return &(wireHit.front());
  }
}

CDCRLWireHit CDCSimHitLookUp::getRLWireHit(const CDCHit* ptrHit,
                                           const std::vector<CDCWireHit>& wireHits) const
{
  ERightLeft rlInfo = getRLInfo(ptrHit);
  double driftLength = getDriftLength(ptrHit);
  const CDCWireHit* wireHit = getWireHit(ptrHit, wireHits);
  B2ASSERT("Could not find CDCWireHit for the requested hit", wireHit);
  return CDCRLWireHit(wireHit, rlInfo, driftLength, CDCWireHit::c_simpleDriftLengthVariance);
}

CDCRecoHit3D CDCSimHitLookUp::getRecoHit3D(const CDCHit* ptrHit,
                                           const std::vector<CDCWireHit>& wireHits) const
{
  CDCRLWireHit rlWireHit = getRLWireHit(ptrHit, wireHits);
  double driftLength = getDriftLength(ptrHit);
  rlWireHit.setRefDriftLength(driftLength);
  Vector3D recoPos3D = getRecoPos3D(ptrHit);
  return CDCRecoHit3D(rlWireHit, recoPos3D);
}

CDCRecoHit3D
CDCSimHitLookUp::getClosestPrimaryRecoHit3D(const CDCHit* ptrHit,
                                            const std::vector<CDCWireHit>& wireHits) const
{
  CDCRLWireHit rlWireHit = getRLWireHit(ptrHit, wireHits);
  double driftLength = getClosestPrimaryDriftLength(ptrHit);
  rlWireHit.setRefDriftLength(driftLength);
  Vector3D recoPos3D = getClosestPrimaryRecoPos3D(ptrHit);
  return CDCRecoHit3D(rlWireHit, recoPos3D);
}

CDCRecoHit2D CDCSimHitLookUp::getRecoHit2D(const CDCHit* ptrHit,
                                           const std::vector<CDCWireHit>& wireHits) const
{
  return getRecoHit3D(ptrHit, wireHits).getRecoHit2D();
}

CDCRecoHit2D
CDCSimHitLookUp::getClosestPrimaryRecoHit2D(const CDCHit* ptrHit,
                                            const std::vector<CDCWireHit>& wireHits) const
{
  return getClosestPrimaryRecoHit3D(ptrHit, wireHits).getRecoHit2D();
}
