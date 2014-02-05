/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCHitLookUp.h"

#include <framework/datastore/RelationVector.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

//ClassImpInCDCLocalTracking(CDCMCHitLookUp)


namespace {



}


namespace {
  CDCMCHitLookUp* g_mcHitLookUp = nullptr;
}



CDCMCHitLookUp::CDCMCHitLookUp()
{
}



CDCMCHitLookUp::~CDCMCHitLookUp()
{
}



CDCMCHitLookUp& CDCMCHitLookUp::getInstance()
{
  if (not g_mcHitLookUp) g_mcHitLookUp = new CDCMCHitLookUp;
  return *g_mcHitLookUp;
}



void CDCMCHitLookUp::fill()
{

  B2INFO("In CDCMCHitLookUp::fill()");

  m_primarySimHits.clear();
  m_rightLeftInfos.clear();
  m_inTrackIds.clear();



  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  std::vector< std::pair<int, int> > mcParticleIdx_simHitTrackId_pairs;
  std::map<int, std::vector<const CDCHit*> > mcHitTracks_by_mcParticleIdx;

  for (const CDCWireHit & wireHit : wireHitTopology.getWireHits()) {

    const CDCHit* ptrHit = wireHit.getHit();
    if (not ptrHit) {
      B2ERROR("CDCWireHit has no containing CDCHit in CDCMCHitLookUp::fill()");
      continue;
    }
    const CDCHit& hit = *ptrHit;



    const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
    if (not ptrSimHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCMCHitLookUp::fill()");
      continue;
    }
    const CDCSimHit& simHit = *ptrSimHit;



    const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>();

    if (mcParticles.size() > 1) {
      B2ERROR("CDCHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;
    } else if (mcParticles.size() == 0) {
      //CDCHit is background
      //Check if background flag is set
      if (simHit.getBackgroundTag() == CDCSimHit::bg_none) {
        B2ERROR("CDCHit has no MCParticle but the related CDCSimHit is not marked as background.");
      }
      continue; //for CDCWireHit
    }
    const MCParticle* ptrMCParticle = mcParticles.object(0);
    const MCParticle& mcParticle = *ptrMCParticle;
    double mcRelationWeight = mcParticles.weight(0);



    const RelationVector<MCParticle> mcParticlesFromSimHit = simHit.getRelationsWith<MCParticle>();

    if (mcParticlesFromSimHit.size() > 1) {
      B2ERROR("CDCSimHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;

    } else if (mcParticlesFromSimHit.size() == 0) {
      B2ERROR("CDCSimHit has no related MCParticle but the CDCHit as a related MCParticle in CDCMCHitLookUp::fill()");
      continue;

    }
    const MCParticle* ptrMCParticleFromSimHit = mcParticlesFromSimHit.object(0);
    double mcRelationWeightFromSimHit =  mcParticlesFromSimHit.weight(0);

    if (ptrMCParticle != ptrMCParticleFromSimHit) {
      B2ERROR("MCParticle from CDCSimHit and CDCHit mismatch in CDCMCHitLookUp::fill()");
      continue;
    }

    if (mcRelationWeight != mcRelationWeightFromSimHit) {
      B2WARNING("The relation weights from SimHit and CDCHit to MCParticle mismatch in CDCMCHitLookUp::fill()");
      B2WARNING("mcRelationWeight: " << mcRelationWeight);
      B2WARNING("mcRelationWeightFromSimHit: " << mcRelationWeightFromSimHit);
      if (mcRelationWeight * mcRelationWeightFromSimHit < 0) {
        B2ERROR("The relation weights from SimHit and CDCHit to MCParticle have mismatching signs in CDCMCHitLookUp::fill()");
        continue;
      }
    }

    m_primarySimHits[ptrHit] = getClosestPrimarySimHit(ptrSimHit);

    int mcParticleIdx = mcParticle.getArrayIndex();
    //Append hit to its own track
    mcHitTracks_by_mcParticleIdx[mcParticleIdx].push_back(ptrHit);

    int simHitTrackId = simHit.getTrackId();
    mcParticleIdx_simHitTrackId_pairs.emplace_back(simHitTrackId, mcParticleIdx);

    //Also consider
    //getStatus, getSecondaryPhysicsProcess
    //for additional information

  } //end for wire hits


  arrangeMCHitTracks(mcHitTracks_by_mcParticleIdx);
  fillInTrackId(mcHitTracks_by_mcParticleIdx);
  fillRLInfo(mcHitTracks_by_mcParticleIdx);


  // Look at the relation of the MCParticle::getArrayIndex() and CDCSimHit::getTrackId()
  std::sort(mcParticleIdx_simHitTrackId_pairs.begin(), mcParticleIdx_simHitTrackId_pairs.end());

  mcParticleIdx_simHitTrackId_pairs.erase(
    std::unique(mcParticleIdx_simHitTrackId_pairs.begin(),
                mcParticleIdx_simHitTrackId_pairs.end()),
    mcParticleIdx_simHitTrackId_pairs.end());

  for (const pair<int, int> mcParticleIdx_simHitTrackId_pair : mcParticleIdx_simHitTrackId_pairs) {

    B2DEBUG(100, "mcParticleIdx <-> simHitTrackId: " <<  mcParticleIdx_simHitTrackId_pair.first << " <-> " << mcParticleIdx_simHitTrackId_pair.second);

  }

}



void CDCMCHitLookUp::arrangeMCHitTracks(std::map<int, std::vector<const CDCHit*> >&  mcHitTracks_by_mcParticleIdx)
{

  for (std::pair<const int, std::vector<const CDCHit*> >& mcHitTrack_and_mcParticleIdx : mcHitTracks_by_mcParticleIdx) {

    //int mcParticleIdx = mcHitTrack_and_mcParticleIdx.first;
    std::vector<const CDCHit*>& mcHitTrack = mcHitTrack_and_mcParticleIdx.second;

    std::sort(mcHitTrack.begin(), mcHitTrack.end(), [this](const CDCHit * hit, const CDCHit * otherHit) -> bool {

      const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
      const CDCSimHit* otherSimHit = otherHit->getRelated<CDCSimHit>();

      const CDCSimHit* primarySimHit = getClosestPrimarySimHit(hit);
      const CDCSimHit* otherPrimarySimHit = getClosestPrimarySimHit(otherHit);

      double primaryFlightTime = primarySimHit ? primarySimHit->getFlightTime() : simHit->getFlightTime();
      double otherPrimaryFlightTime = otherPrimarySimHit ? otherPrimarySimHit->getFlightTime() : otherSimHit->getFlightTime();

      double secondaryFlightTime =  simHit->getFlightTime();
      double otherSecondaryFlightTime =  otherSimHit->getFlightTime();

      return primaryFlightTime < otherPrimaryFlightTime or
      (primaryFlightTime == otherPrimaryFlightTime and
      secondaryFlightTime < otherSecondaryFlightTime);

    });
  }
}



void CDCMCHitLookUp::fillInTrackId(std::map<int, std::vector<const CDCHit*> >&  mcHitTracks_by_mcParticleIdx)
{

  for (std::pair<const int, std::vector<const CDCHit*> >& mcHitTrack_and_mcParticleIdx : mcHitTracks_by_mcParticleIdx) {

    std::vector<const CDCHit*>& mcHitTrack = mcHitTrack_and_mcParticleIdx.second;

    //Fill the in track ids
    int iHit = -1;
    for (const CDCHit * ptrHit : mcHitTrack) {
      ++iHit;
      m_inTrackIds[ptrHit] = iHit;
    }
  }
}


void CDCMCHitLookUp::fillRLInfo(std::map<int, std::vector<const CDCHit*> >&  mcHitTracks_by_mcParticleIdx)
{

  for (std::pair<int, std::vector<const CDCHit*> > mcHitTrack_and_mcParticleIdx : mcHitTracks_by_mcParticleIdx) {

    std::vector<const CDCHit*>& mcHitTrack = mcHitTrack_and_mcParticleIdx.second;

    //Fill the right left passage hypothese
    for (const CDCHit * ptrHit : mcHitTrack) {

      if (ptrHit) {
        const CDCSimHit* ptrSimHit = ptrHit->getRelated<CDCSimHit>();
        if (not ptrSimHit) {
          m_rightLeftInfos[ptrHit] = INVALID_INFO;
        } else {
          const CDCSimHit& simHit = *ptrSimHit;
          m_rightLeftInfos[ptrHit] = getPrimaryRLInfo(simHit);
        }
      }

    }

  }

}


RightLeftInfo CDCMCHitLookUp::getRLInfo(const CDCSimHit& simHit) const
{

  // find out if the wire is right or left of the track ( view in flight direction )
  Vector3D trackPosToWire =  simHit.getPosWire();
  trackPosToWire.subtract(simHit.getPosTrack());

  Vector3D directionOfFlight = simHit.getMomentum();
  RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());

  return rlInfo;

}

RightLeftInfo CDCMCHitLookUp::getPrimaryRLInfo(const CDCSimHit& simHit) const
{

  const CDCSimHit* ptrClosestPrimarySimHit = getClosestPrimarySimHit(&simHit);
  if (ptrClosestPrimarySimHit) {
    const CDCSimHit& closestPrimarySimHit = *ptrClosestPrimarySimHit;

    // find out if the wire is right or left of the track ( view in flight direction )
    Vector3D trackPosToWire =  simHit.getPosWire();
    trackPosToWire.subtract(simHit.getPosTrack());

    //Take the momentum of the primary hit in this case
    Vector3D directionOfFlight = closestPrimarySimHit.getMomentum();

    RightLeftInfo rlInfo = trackPosToWire.xy().isRightOrLeftOf(directionOfFlight.xy());
    return rlInfo;

  } else {
    return INVALID_INFO;
  }

}


const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCSimHit* ptrSimHit) const
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

      B2INFO("Found primary hit for reassigned secondary");
      return *itClosestPrimarySimHit;

    } else {

      B2WARNING("NO primary hit for reassigned secondary");
      return nullptr;
    }
  }

}




bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCHit& hit) const
{

  const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>();

  if (mcParticles.size() == 1) {
    return  mcParticles.weight(0) <= 0.0;
  } else {
    return false;
  }

}



bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCSimHit& simHit) const
{
  const RelationVector<MCParticle> mcParticles = simHit.getRelationsWith<MCParticle>();
  if (mcParticles.size() == 1) {
    return  mcParticles.weight(0) <= 0.0;
  } else {
    return false;
  }
}



const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCHit* hit) const
{

  auto itFoundPrimarySimHit = m_primarySimHits.find(hit);
  return itFoundPrimarySimHit == m_primarySimHits.end() ? nullptr : itFoundPrimarySimHit->second;

}


bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  return hit ? isReassignedSecondaryHit(*hit) : false;

}



const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  return getClosestPrimarySimHit(hit);

}



ITrackType CDCMCHitLookUp::getMCTrackId(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  const MCParticle* mcParticle = hit->getRelated<MCParticle>();
  return mcParticle ? mcParticle->getArrayIndex() : INVALID_ITRACK;

}




bool CDCMCHitLookUp::isBackground(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();
  const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
  return simHit->getBackgroundTag() != CDCSimHit::bg_none;

}



int CDCMCHitLookUp::getInTrackId(const CDCWireHit& wireHit) const
{

  const CDCHit* hit = wireHit.getHit();

  auto itFoundHit = m_inTrackIds.find(hit);

  return itFoundHit == m_inTrackIds.end() ? -999 : itFoundHit->second;

}



RightLeftInfo CDCMCHitLookUp::getRLInfo(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();

  auto itFoundHit = m_rightLeftInfos.find(hit);

  return itFoundHit == m_rightLeftInfos.end() ?  INVALID_INFO : itFoundHit->second;

}



bool CDCMCHitLookUp::isCorrect(const CDCRLWireHitTriple& rlWireHitTriple, int inTrackHitDistanceTolerance) const
{

  const CDCWireHit& startWireHit = rlWireHitTriple.getStartWireHit();
  const CDCWireHit& middleWireHit = rlWireHitTriple.getMiddleWireHit();
  const CDCWireHit& endWireHit = rlWireHitTriple.getEndWireHit();

  //First check if the track ids are in agreement
  ITrackType startMCTrackId = getMCTrackId(startWireHit);
  ITrackType middleMCTrackId = getMCTrackId(middleWireHit);
  ITrackType endMCTrackId = getMCTrackId(endWireHit);

  if (not(startMCTrackId == middleMCTrackId and middleMCTrackId == endMCTrackId) or startMCTrackId == INVALID_ITRACK) {
    return false;
  }





  //Maybe be a bit more permissive for reassigned hits
  bool startIsReassigned = isReassignedSecondaryHit(startWireHit);
  bool middleIsReassigned = isReassignedSecondaryHit(middleWireHit);
  bool endIsReassigned = isReassignedSecondaryHit(endWireHit);





  //Now check the alignement in track
  bool distanceInTrackIsSufficientlyLow = true;

  int startInTrackId = getInTrackId(startWireHit);
  int middleInTrackId = getInTrackId(middleWireHit);
  int endInTrackId = getInTrackId(endWireHit);

  int startToMiddleInTrackDistance =  middleInTrackId - startInTrackId;
  int middleToEndInTrackDistance =  endInTrackId - middleInTrackId;
  int startToEndInTrackDistance =  endInTrackId - startInTrackId;

  if (true or (not startIsReassigned and not middleIsReassigned and not endIsReassigned)) {

    distanceInTrackIsSufficientlyLow =
      0 < startToMiddleInTrackDistance and startToMiddleInTrackDistance <= inTrackHitDistanceTolerance and
      0 < middleToEndInTrackDistance and middleToEndInTrackDistance <= inTrackHitDistanceTolerance;

    // if (not distanceInTrackIsSufficientlyLow){
    //   B2WARNING("Disagreement here");
    //   B2WARNING("startToMiddleInTrackDistance: " <<  startToMiddleInTrackDistance);
    //   B2WARNING("middleToEndInTrackDistance: " <<  middleToEndInTrackDistance);
    // }

  } else if (not startIsReassigned and not middleIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < startToMiddleInTrackDistance and startToMiddleInTrackDistance <= inTrackHitDistanceTolerance;

  } else if (not middleIsReassigned and not endIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < middleToEndInTrackDistance and middleToEndInTrackDistance <= inTrackHitDistanceTolerance;

  } else if (not startIsReassigned and not endIsReassigned) {

    distanceInTrackIsSufficientlyLow =
      0 < startToEndInTrackDistance and  startToEndInTrackDistance <= 2 * inTrackHitDistanceTolerance;

  } else {

    // can not say anything about the in

  }

  if (not distanceInTrackIsSufficientlyLow) return false;


  //Now check the right left information in track
  RightLeftInfo startRLInfo = rlWireHitTriple.getStartRLInfo();
  RightLeftInfo middleRLInfo = rlWireHitTriple.getMiddleRLInfo();
  RightLeftInfo endRLInfo = rlWireHitTriple.getEndRLInfo();


  RightLeftInfo mcStartRLInfo = getRLInfo(startWireHit);
  RightLeftInfo mcMiddleRLInfo = getRLInfo(middleWireHit);
  RightLeftInfo mcEndRLInfo = getRLInfo(endWireHit);



  if (startRLInfo == mcStartRLInfo and middleRLInfo == mcMiddleRLInfo and endRLInfo == mcEndRLInfo) {
    return true;
  } else {
    //return false;
  }



  const CDCSimHit* startSimHit = getSimHit(startWireHit);
  const CDCSimHit* middleSimHit = getSimHit(middleWireHit);
  const CDCSimHit* endSimHit = getSimHit(endWireHit);

  if (not startSimHit) {
    B2WARNING("Start wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }
  if (not middleSimHit) {
    B2WARNING("Middle wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }
  if (not startSimHit) {
    B2WARNING("End wire hit of an oriented wire hit triple has no related sim hit");
    return false;
  }

  CDCRecoHit2D startMCHit =  CDCRecoHit2D::fromSimHit(&startWireHit, *startSimHit);
  CDCRecoHit2D middleMCHit = CDCRecoHit2D::fromSimHit(&middleWireHit, *middleSimHit);
  CDCRecoHit2D endMCHit = CDCRecoHit2D::fromSimHit(&endWireHit, *endSimHit);

  CDCRecoTangent startToMiddleMCTangent(startMCHit, middleMCHit);
  CDCRecoTangent startToEndMCTangent(startMCHit, endMCHit);
  CDCRecoTangent middleToEndMCTangent(middleMCHit, endMCHit);

  //the right left passage info from mc often is not correct, because the simhits offer poor momentum information (therefor limites flight direction information)
  startToMiddleMCTangent.adjustRLInfo();
  startToEndMCTangent.adjustRLInfo();
  middleToEndMCTangent.adjustRLInfo();

  //check if the tangents have matching right left passage information in their own recohits
  if (startToEndMCTangent.getFromRLInfo() !=
      startToMiddleMCTangent.getFromRLInfo() or

      startToMiddleMCTangent.getToRLInfo() !=
      middleToEndMCTangent.getFromRLInfo() or

      startToEndMCTangent.getToRLInfo() !=
      middleToEndMCTangent.getToRLInfo()) {

    return false;
  }

  if (startWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getStartRLInfo() or
        startToEndMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getStartRLInfo()) return false;
  }

  if (middleWireHit->getRefDriftLength() != 0) {
    if (startToMiddleMCTangent.getToRLInfo() !=
        rlWireHitTriple.getMiddleRLInfo() or

        middleToEndMCTangent.getFromRLInfo() !=
        rlWireHitTriple.getMiddleRLInfo()) return false;
  }

  if (endWireHit->getRefDriftLength() != 0) {
    if (startToEndMCTangent.getToRLInfo() !=
        rlWireHitTriple.getEndRLInfo() or

        middleToEndMCTangent.getToRLInfo() !=
        rlWireHitTriple.getEndRLInfo()) return false;
  }

  return true;

}
