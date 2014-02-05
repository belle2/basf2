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

  m_rightLeftInfos.clear();
  m_inTrackIds.clear();

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  std::vector< std::pair<int, int> > mcParticleIdx_simHitTrackId_pairs;
  std::map<int, std::vector<const CDCHit*> > mcHitTracks_by_mcParticleIdx;

  for (const CDCWireHit & wireHit : wireHitTopology.getWireHits()) {

    const CDCHit* hit = wireHit.getHit();
    if (not hit) {
      B2ERROR("CDCWireHit has no containing CDCHit in CDCMCHitLookUp::fill()");
      continue;
    }

    const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
    if (not simHit) {
      B2ERROR("CDCHit has no related CDCSimHit in CDCMCHitLookUp::fill()");
      continue;
    }

    const RelationVector<MCParticle> mcParticles = hit->getRelationsWith<MCParticle>();
    const MCParticle* mcParticle = nullptr;
    double mcRelationWeight = NAN;

    if (mcParticles.size() > 1) {
      B2ERROR("CDCHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;
    } else if (mcParticles.size() == 0) {
      B2WARNING("CDCHit has no related MCParticle in CDCMCHitLookUp::fill()");
      mcParticle = nullptr;
    } else {
      mcParticle = mcParticles.object(0);
      mcRelationWeight = mcParticles.weight(0);
    }

    const RelationVector<MCParticle> mcParticlesFromSimHit = simHit->getRelationsWith<MCParticle>();
    const MCParticle* mcParticleFromSimHit = nullptr;
    double mcRelationWeightFromSimHit = NAN;

    if (mcParticlesFromSimHit.size() > 1) {
      B2ERROR("CDCSimHit has more than one related MCParticle in CDCMCHitLookUp::fill()");
      continue;
    } else if (mcParticlesFromSimHit.size() == 0) {
      B2WARNING("CDCSimHit has no related MCParticle in CDCMCHitLookUp::fill()");
      mcParticleFromSimHit = nullptr;
    } else {
      mcParticleFromSimHit = mcParticlesFromSimHit.object(0);
      mcRelationWeightFromSimHit = mcParticlesFromSimHit.weight(0);
    }


    if (mcParticle != mcParticleFromSimHit) {
      B2WARNING("MCParticle from CDCSimHit and CDCHit mismatch in CDCMCHitLookUp::fill()");
      continue;
    }

    if (mcRelationWeight != mcRelationWeightFromSimHit) {
      B2WARNING("The relation weights from SimHit and CDCHit to MCParticle mismatch in CDCMCHitLookUp::fill()");
      B2WARNING("mcRelationWeight: " << mcRelationWeight);
      B2WARNING("mcRelationWeightFromSimHit: " << mcRelationWeightFromSimHit);
      continue;
    }



    bool isBackground = not mcParticle;
    if (isBackground) {
      //Check if background flag is set
      if (simHit->getBackgroundTag() == CDCSimHit::bg_none) {
        B2ERROR("CDCHit has no MCParticle but the related CDCSimHit is not marked as background.");
      }
    } else {
      int mcParticleIdx = mcParticle->getArrayIndex();
      int simHitTrackId = simHit->getTrackId();
      mcParticleIdx_simHitTrackId_pairs.emplace_back(simHitTrackId, mcParticleIdx);

      //Append hit to its own hit
      mcHitTracks_by_mcParticleIdx[mcParticleIdx].push_back(hit);

      //Also consider
      //getStatus, getSecondaryPhysicsProcess
      //for additional information


    }
  } //end for wire hits


  for (std::pair<int, std::vector<const CDCHit*> > mcHitTrack_and_mcParticleIdx : mcHitTracks_by_mcParticleIdx) {

    int mcParticleIdx = mcHitTrack_and_mcParticleIdx.first;
    std::vector<const CDCHit*> mcHitTrack = mcHitTrack_and_mcParticleIdx.second;

    std::sort(mcHitTrack.begin(), mcHitTrack.end(), [](const CDCHit * hit, const CDCHit * otherHit) {
      const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
      const CDCSimHit* otherSimHit = otherHit->getRelated<CDCSimHit>();
      return simHit->getFlightTime() < otherSimHit->getFlightTime() or
             (simHit->getFlightTime() == otherSimHit->getFlightTime()
              and hit->getArrayIndex() < otherHit->getArrayIndex());
    });

    //Fill the in track ids and the right left passage hypothese
    int iHit = -1;
    for (const CDCHit * hit : mcHitTrack) {
      ++iHit;
      m_inTrackIds[hit] = iHit;

      const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
      B2INFO("PosFlag: " << simHit->getPosFlag());
      m_rightLeftInfos[hit] = simHit->getPosFlag();

    }

    //Check if the mcTracks are time of flight aligned
    bool isTimeOfFlightAligned = std::is_sorted(mcHitTrack.begin(), mcHitTrack.end(), [](const CDCHit * hit, const CDCHit * otherHit) {
      const CDCSimHit* simHit = hit->getRelated<CDCSimHit>();
      const CDCSimHit* otherSimHit = otherHit->getRelated<CDCSimHit>();
      return simHit->getFlightTime() < otherSimHit->getFlightTime();
    });

    if (not isTimeOfFlightAligned) {
      //is the case all the time
      B2DEBUG(100, "Monte Carlo track to MCParticle with index " << mcParticleIdx << " is not time of flight aligned.");
      B2DEBUG(100, "Hit content:");
      for (const CDCHit * hit : mcHitTrack) {

        B2DEBUG(100,
                "Hit idx:" << hit->getArrayIndex() << ", " <<
                "isReassigned: " << isReassignedSecondaryHit(*hit) << ", " <<
                "FlightTime(): " << hit->getRelated<CDCSimHit>()->getFlightTime() << ", " <<
                "GlobalTime(): " << hit->getRelated<CDCSimHit>()->getGlobalTime());
      }

    }


  }

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


bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCHit& hit) const
{

  const RelationVector<MCParticle> mcParticles = hit.getRelationsWith<MCParticle>();

  if (mcParticles.size() == 1) {
    return  mcParticles.weight(0) <= 0.0;
  } else {
    return false;
  }

}



bool CDCMCHitLookUp::isReassignedSecondaryHit(const CDCWireHit& wireHit) const
{
  const CDCHit* hit = wireHit.getHit();
  return hit ? isReassignedSecondaryHit(*hit) : false;

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
