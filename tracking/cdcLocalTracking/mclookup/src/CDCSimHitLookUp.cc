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

#include <framework/datastore/RelationVector.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>

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

  for (const CDCHit & hit : hits) {

    const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
    if (not ptrSimHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCSimHitLookUp::fill()");
      continue;
    }
    //const CDCSimHit& simHit = *ptrSimHit;

    const CDCHit* ptrHit = &hit;

    m_primarySimHits[ptrHit] = getClosestPrimarySimHit(ptrSimHit);

  }

}



bool CDCSimHitLookUp::isReassignedSecondaryHit(const CDCSimHit& simHit) const
{
  const RelationVector<MCParticle> mcParticles = simHit.getRelationsWith<MCParticle>();
  if (mcParticles.size() == 1) {
    return  mcParticles.weight(0) <= 0.0;
  } else {
    return false;
  }
}



const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCSimHit* ptrSimHit) const
{

  if (not ptrSimHit) {
    return nullptr;
  }
  const CDCSimHit& simHit = *ptrSimHit;

  //Check if the CDCSimHit was reassigned from a secondary particle to its primary particle
  if (not isReassignedSecondaryHit(simHit)) {
    return ptrSimHit;
  } else {

    //Try to find the hit on the same wire from the primary particle
    const MCParticle* ptrMCParticle = simHit.getRelated<MCParticle>();
    if (not ptrMCParticle) {
      return nullptr;
    }

    const MCParticle& mcParticle = *ptrMCParticle;

    //Get all CDCSimHits that are related with the MCParticle
    const RelationVector<CDCSimHit> simHitsOfMCParticle = mcParticle.getRelationsWith<CDCSimHit>();

    size_t nSimHitsOfMCParticle = simHitsOfMCParticle.size();

    WireID wireID = simHit.getWireID();
    std::vector<const CDCSimHit*> primarySimHitsOnSameOrNeighborWire;

    for (size_t iSimHit = 0; iSimHit < nSimHitsOfMCParticle; ++iSimHit) {
      if ((CDCWireTopology::getInstance().areNeighbors(simHitsOfMCParticle.object(iSimHit)->getWireID(), wireID) or
           simHitsOfMCParticle.object(iSimHit)->getWireID() == wireID) and
          simHitsOfMCParticle.weight(iSimHit) > 0) {
        // Found a hit on the same wire from the primary particle
        primarySimHitsOnSameOrNeighborWire.push_back(simHitsOfMCParticle.object(iSimHit));
      }
    }

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



void CDCSimHitLookUp::fillRLInfo()
{
  StoreArray<CDCHit> hits;
  for (const CDCHit & hit : hits) {

    const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
    const CDCHit* ptrHit = &hit;

    if (not ptrSimHit) {
      m_rightLeftInfos[ptrHit] = INVALID_INFO;
    } else {

      const CDCSimHit& simHit = *ptrSimHit;
      m_rightLeftInfos[ptrHit] = getPrimaryRLInfo(simHit);
    }

  }

}



RightLeftInfo CDCSimHitLookUp::getPrimaryRLInfo(const CDCSimHit& simHit) const
{

  const CDCSimHit* ptrClosestPrimarySimHit = getClosestPrimarySimHit(&simHit);
  if (ptrClosestPrimarySimHit) {
    const CDCSimHit& closestPrimarySimHit = *ptrClosestPrimarySimHit;

    //Take the momentum of the primary hit in this case
    Vector3D directionOfFlight = closestPrimarySimHit.getMomentum();

    // find out if the wire is right or left of the track ( view in flight direction )
    Vector3D trackPosToWire =  simHit.getPosWire();
    trackPosToWire.subtract(simHit.getPosTrack());

    RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());
    return rlInfo;

  } else {
    return INVALID_INFO;
  }

}


/*
RightLeftInfo CDCSimHitLookUp::getRLInfo(const CDCSimHit& simHit) const
{

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simHit.getPosWire();
  trackPosToWire.subtract(simHit.getPosTrack());

  Vector3D directionOfFlight = simHit.getMomentum();
  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  return rlInfo;

  }*/



bool CDCSimHitLookUp::isReassignedSecondaryHit(const CDCHit* ptrHit) const
{

  if (not ptrHit) return false;

  const CDCHit& hit = *ptrHit;

  const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>();

  if (mcParticles.size() == 1) {
    return  mcParticles.weight(0) <= 0.0;
  } else {
    return false;
  }

}



const CDCSimHit* CDCSimHitLookUp::getClosestPrimarySimHit(const CDCHit* hit) const
{
  auto itFoundPrimarySimHit = m_primarySimHits.find(hit);
  return itFoundPrimarySimHit == m_primarySimHits.end() ? nullptr : itFoundPrimarySimHit->second;
}



RightLeftInfo CDCSimHitLookUp::getRLInfo(const CDCHit* hit) const
{

  auto itFoundHit = m_rightLeftInfos.find(hit);
  return itFoundHit == m_rightLeftInfos.end() ?  INVALID_INFO : itFoundHit->second;

}


