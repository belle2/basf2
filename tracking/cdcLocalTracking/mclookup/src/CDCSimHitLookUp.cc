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

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCMap.h>

#include <framework/datastore/StoreArray.h>

#include <vector>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCSimHitLookUp)

namespace {
  CDCSimHitLookUp* g_simHitLookUp = nullptr;
}



CDCSimHitLookUp::CDCSimHitLookUp()
{
}



CDCSimHitLookUp::~CDCSimHitLookUp()
{
}



CDCSimHitLookUp& CDCSimHitLookUp::getInstance()
{

  if (not g_simHitLookUp) g_simHitLookUp = new CDCSimHitLookUp;
  return *g_simHitLookUp;

}



void CDCSimHitLookUp::clear()
{

  m_primarySimHits.clear();
  m_rightLeftInfos.clear();

}



void CDCSimHitLookUp::fill()
{

  B2DEBUG(100, "In CDCSimHitLookUp::fill()");
  clear();

  fillPrimarySimHits();
  fillRLInfo();

  B2INFO("m_primarySimHits.size(): " << m_primarySimHits.size());
  B2INFO("m_rightLeftInfos.size(): " << m_rightLeftInfos.size());
}



void CDCSimHitLookUp::fillPrimarySimHits()
{
  StoreArray<CDCHit> hits;

  const CDCMCMap& mcMap = CDCMCMap::getInstance();

  for (const CDCHit & hit : hits) {

    const CDCHit* ptrHit = &hit;
    const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);

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

  const CDCMCMap& mcMap = CDCMCMap::getInstance();

  //Check if the CDCSimHit was reassigned from a secondary particle to its primary particle
  if (not mcMap.isReassignedSecondary(ptrSimHit)) {
    return ptrSimHit;
  } else {

    //Try to find the hit on the same wire from the primary particle
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

        // Found a hit on the same wire from the primary particle
        primarySimHitsOnSameOrNeighborWire.push_back(ptrPrimarySimHit);
      }
    }


    //Now from the neighboring primary CDCSimHits pick to one with the smallest distance to the secondary CDCSimHit
    auto itClosestPrimarySimHit = std::min_element(primarySimHitsOnSameOrNeighborWire.begin(),
                                                   primarySimHitsOnSameOrNeighborWire.end(),
    [&simHit](const CDCSimHit * primarySimHit, const CDCSimHit * otherPrimarySimHit) -> bool {

      Vector3D primaryHitPos = primarySimHit->getPosTrack();
      Vector3D otherPrimaryHitPos = otherPrimarySimHit->getPosTrack();

      Vector3D secondaryHitPos = simHit.getPosTrack();

      return primaryHitPos.distance(secondaryHitPos) < otherPrimaryHitPos.distance(secondaryHitPos);

    });

    if (itClosestPrimarySimHit != primarySimHitsOnSameOrNeighborWire.end()) {

      //B2INFO("Found primary hit for reassigned secondary");
      return *itClosestPrimarySimHit;

    } else {

      B2WARNING("NO primary hit for reassigned secondary");
      return nullptr;
    }
  }

}



const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCHit* ptrHit) const
{
  const CDCMCMap& mcMap = CDCMCMap::getInstance();
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

  const CDCMCMap& mcMap = CDCMCMap::getInstance();

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
  StoreArray<CDCHit> hits;
  const CDCMCMap& mcMap = CDCMCMap::getInstance();

  for (const CDCHit & hit : hits) {

    const CDCHit* ptrHit = &hit;

    const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);
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








