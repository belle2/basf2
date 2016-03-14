/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCSimHitLookUp.h"
#include "../include/CDCMCManager.h"

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>
#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#include <vector>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


CDCSimHitLookUp::CDCSimHitLookUp()
{
}



CDCSimHitLookUp::~CDCSimHitLookUp()
{
}


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

  for (const CDCMCMap::CDCSimHitByCDCHitRelation & relation : mcMap.getSimHitByHitRelations()) {

    const CDCHit* ptrHit = relation.get<CDCHit>();
    const CDCSimHit* ptrSimHit = relation.get<CDCSimHit>();

    if (not ptrSimHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCSimHitLookUp::fill()");
      continue;
    }

    if (mcMap.isReassignedSecondary(ptrSimHit)) {
      m_primarySimHits[ptrHit] = getClosestPrimarySimHit(ptrSimHit);
    }
  }

}



const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCSimHit* ptrSimHit) const
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

    for (const CDCMCMap::CDCSimHitByMCParticleRelation & simHitByMCParticleRelation : mcMap.getSimHits(ptrMCParticle)) {

      const CDCSimHit* ptrPrimarySimHit = simHitByMCParticleRelation.get<CDCSimHit>();
      if (mcMap.isReassignedSecondary(ptrPrimarySimHit) or not ptrPrimarySimHit) continue;

      const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

      if (wireTopology.areNeighbors(primarySimHit.getWireID(), wireID) or primarySimHit.getWireID() == wireID) {

        // Found a hit on the same wire from the primary particle.
        primarySimHitsOnSameOrNeighborWire.push_back(ptrPrimarySimHit);
      }
    }


    // Now from the neighboring primary CDCSimHits pick to one with the smallest distance to the secondary CDCSimHit.
    auto itClosestPrimarySimHit = std::min_element(primarySimHitsOnSameOrNeighborWire.begin(),
                                                   primarySimHitsOnSameOrNeighborWire.end(),
    [&simHit](const CDCSimHit * primarySimHit, const CDCSimHit * otherPrimarySimHit) -> bool {

      Vector3D primaryHitPos = primarySimHit->getPosTrack();
      Vector3D otherPrimaryHitPos = otherPrimarySimHit->getPosTrack();

      Vector3D secondaryHitPos = simHit.getPosTrack();

      return primaryHitPos.distance(secondaryHitPos) < otherPrimaryHitPos.distance(secondaryHitPos);

    });

    if (itClosestPrimarySimHit != primarySimHitsOnSameOrNeighborWire.end()) {
      // Found primary simulated hit for secondary hit.
      return *itClosestPrimarySimHit;

    } else {
      B2WARNING("NO primary hit for reassigned secondary");
      return nullptr;

    }
  }

}



const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCHit* ptrHit) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Look up closest primary sim hit.")
    return nullptr;
  }
  const CDCMCMap& mcMap = *m_ptrMCMap;

  if (mcMap.isReassignedSecondary(ptrHit)) {

    auto itFoundPrimarySimHit = m_primarySimHits.find(ptrHit);
    return itFoundPrimarySimHit == m_primarySimHits.end() ? nullptr : itFoundPrimarySimHit->second;

  } else {
    return mcMap.getSimHit(ptrHit);
  }

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

  const CDCSimHit* ptrPrimarySimHit =  mcMap.isReassignedSecondary(ptrHit) ? getClosestPrimarySimHit(ptrHit) : ptrSimHit;

  if (not ptrPrimarySimHit) {
    // if no primary simhit is close to the secondary hit we can only take the secondary
    ptrPrimarySimHit = ptrSimHit;

    //or invent something better at some point...
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  //Take the momentum of the primary hit
  Vector3D directionOfFlight = primarySimHit.getMomentum();
  return directionOfFlight;

}



void CDCSimHitLookUp::fillRLInfo()
{

  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot setup right left passage information map");
    return;
  }
  const CDCMCMap& mcMap = *m_ptrMCMap;

  for (const CDCMCMap::CDCSimHitByCDCHitRelation & relation : mcMap.getSimHitByHitRelations()) {

    const CDCHit* ptrHit = relation.get<CDCHit>();
    const CDCSimHit* ptrSimHit = relation.get<CDCSimHit>();

    if (not ptrSimHit) continue;
    const CDCSimHit& simHit = *ptrSimHit;

    Vector3D directionOfFlight = getDirectionOfFlight(ptrHit);
    if (directionOfFlight.isNull()) continue;

    // find out if the wire is right or left of the track ( view in flight direction )
    Vector3D trackPosToWire =  simHit.getPosWire();
    trackPosToWire.subtract(simHit.getPosTrack());

    RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());
    m_rightLeftInfos[ptrHit] = rlInfo;

  }

}



RightLeftInfo CDCSimHitLookUp::getRLInfo(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_rightLeftInfos.find(ptrHit);
  return itFoundHit == m_rightLeftInfos.end() ?  INVALID_INFO : itFoundHit->second;

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
  return simHit.getPosTrack();
}


Vector3D CDCSimHitLookUp::getClosestPrimaryRecoPos3D(const CDCHit* ptrHit) const
{
  const CDCSimHit* ptrPrimarySimHit =  getClosestPrimarySimHit(ptrHit);
  if (ptrPrimarySimHit) {
    const CDCSimHit& primarySimHit = *ptrPrimarySimHit;
    return primarySimHit.getPosTrack();
  } else {
    return getRecoPos3D(ptrHit);
  }
}


const CDCRLWireHit* CDCSimHitLookUp::getRLWireHit(const CDCHit* ptrHit) const
{
  if (not ptrHit) return nullptr;

  RightLeftInfo rlInfo = getRLInfo(ptrHit);

  const CDCWireHitTopology& theWireHitTopology = CDCWireHitTopology::getInstance();
  const CDCRLWireHit* ptrRLWireHit = theWireHitTopology.getRLWireHit(ptrHit, rlInfo);

  return ptrRLWireHit;
}


CDCRecoHit3D CDCSimHitLookUp::getRecoHit3D(const CDCHit* ptrHit) const
{
  const CDCRLWireHit* ptrRLWireHit = getRLWireHit(ptrHit);
  if (not ptrRLWireHit) return CDCRecoHit3D();

  Vector3D recoPos3D = getRecoPos3D(ptrHit);

  return CDCRecoHit3D(ptrRLWireHit, recoPos3D);
}



CDCRecoHit3D CDCSimHitLookUp::getClosestPrimaryRecoHit3D(const CDCHit* ptrHit) const
{
  const CDCRLWireHit* ptrRLWireHit = getRLWireHit(ptrHit);
  if (not ptrRLWireHit) return CDCRecoHit3D();

  Vector3D recoPos3D = getClosestPrimaryRecoPos3D(ptrHit);

  return CDCRecoHit3D(ptrRLWireHit, recoPos3D);
}



CDCRecoHit2D CDCSimHitLookUp::getRecoHit2D(const CDCHit* ptrHit) const
{
  return getRecoHit3D(ptrHit).getRecoHit2D();
}



CDCRecoHit2D CDCSimHitLookUp::getClosestPrimaryRecoHit2D(const CDCHit* ptrHit) const
{
  return getClosestPrimaryRecoHit3D(ptrHit).getRecoHit2D();
}
