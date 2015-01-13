/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCSegmentLookUp.h"
#include "../include/CDCMCManager.h"

#include <TDatabasePDG.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCMCSegmentLookUp)


CDCMCSegmentLookUp::CDCMCSegmentLookUp()
{
}





CDCMCSegmentLookUp::~CDCMCSegmentLookUp()
{
}





const CDCMCSegmentLookUp& CDCMCSegmentLookUp::getInstance()
{
  return CDCMCManager::getMCSegmentLookUp();
}





void CDCMCSegmentLookUp::clear()
{
  B2DEBUG(100, "Clearing CDCMCSegmentLookUp");
  m_mcTrackIds.clear();
  B2DEBUG(100, "m_mcTrackIds.size(): " <<  m_mcTrackIds.size());
}





const float CDCMCSegmentLookUp::s_minimalMatchPurity = 0.5;




ITrackType CDCMCSegmentLookUp::getMCTrackId(const CDCRecoSegment2D* ptrSegment2D) const
{

  if (not ptrSegment2D) return INVALID_ITRACK;

  const CDCRecoSegment2D& segment2D = *ptrSegment2D;
  auto itFound = m_mcTrackIds.find(ptrSegment2D);

  if (itFound == m_mcTrackIds.end()) {
    MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(segment2D);
    if (mcTrackIdAndPurity.getPurity() >= s_minimalMatchPurity) {
      m_mcTrackIds[ptrSegment2D] = mcTrackIdAndPurity.getMCTrackId();
      return mcTrackIdAndPurity.getMCTrackId();
    } else {
      m_mcTrackIds[ptrSegment2D] = INVALID_ITRACK;
      return INVALID_ITRACK;
    }

  } else {
    return itFound->second;
  }
}



const MCParticle* CDCMCSegmentLookUp::getMCParticle(const CDCRecoSegment2D* ptrSegment2D) const
{
  const CDCHit* ptrHit = getFirstHit(ptrSegment2D);
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  return mcHitLookUp.getMCParticle(ptrHit);
}



const CDCHit* CDCMCSegmentLookUp::getFirstHit(const CDCRecoSegment2D* ptrSegment2D) const
{
  if (not ptrSegment2D) return nullptr;
  const CDCRecoSegment2D& segment2D = *ptrSegment2D;

  ITrackType mcTrackId = getMCTrackId(ptrSegment2D);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCRecoHit2D & recoHit2D : segment2D) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(recoHit2D)) return recoHit2D;
  }
  return nullptr;

}





const CDCHit* CDCMCSegmentLookUp::getLastHit(const CDCRecoSegment2D* ptrSegment2D) const
{

  if (not ptrSegment2D) return nullptr;
  const CDCRecoSegment2D& segment2D = *ptrSegment2D;

  ITrackType mcTrackId = getMCTrackId(ptrSegment2D);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCRecoHit2D & recoHit2D : segment2D.reverseRange()) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(recoHit2D)) return recoHit2D;
  }
  return nullptr;

}





ForwardBackwardInfo CDCMCSegmentLookUp::isForwardOrBackwardToMCTrack(const CDCRecoSegment2D* ptrSegment2D) const
{
  Index firstInTrackId = getFirstInTrackId(ptrSegment2D);
  Index lastInTrackId = getLastInTrackId(ptrSegment2D);
  if (firstInTrackId == INVALID_INDEX or lastInTrackId == INVALID_INDEX) return INVALID_INFO;
  else if (firstInTrackId < lastInTrackId) return FORWARD;
  else if (firstInTrackId > lastInTrackId) return BACKWARD;
  else if (firstInTrackId == lastInTrackId) return INVALID_INFO;
  return INVALID_ITRACK;
}





ForwardBackwardInfo CDCMCSegmentLookUp::areAlignedInMCTrack(const CDCRecoSegment2D* ptrStartSegment2D, const CDCRecoSegment2D* ptrEndSegment2D) const
{

  // Check if the segments are sensable on their own
  ForwardBackwardInfo startFBInfo = isForwardOrBackwardToMCTrack(ptrStartSegment2D);
  if (startFBInfo == INVALID_INFO) return INVALID_INFO;

  ForwardBackwardInfo endFBInfo = isForwardOrBackwardToMCTrack(ptrEndSegment2D);
  if (endFBInfo == INVALID_INFO) return INVALID_INFO;

  if (startFBInfo != endFBInfo) return INVALID_INFO;


  /// Check if the segments are in the same track
  ITrackType startMCTrackId = getMCTrackId(ptrStartSegment2D);
  if (startMCTrackId == INVALID_ITRACK) return INVALID_INFO;

  ITrackType endMCTrackId = getMCTrackId(ptrEndSegment2D);
  if (endMCTrackId == INVALID_ITRACK) return INVALID_INFO;

  if (startMCTrackId != endMCTrackId) return INVALID_INFO;


  // Now check are aligned with their common track
  // Examine if they are in the same super layer

  Index lastNPassedSuperLayersOfStartSegment = getLastNPassedSuperLayers(ptrStartSegment2D);
  if (lastNPassedSuperLayersOfStartSegment == INVALID_INDEX) return INVALID_INFO;

  Index firstNPassedSuperLayersOfEndSegment = getFirstNPassedSuperLayers(ptrEndSegment2D);
  if (firstNPassedSuperLayersOfEndSegment == INVALID_INDEX) return INVALID_INFO;

  if (lastNPassedSuperLayersOfStartSegment < firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == FORWARD and  endFBInfo == FORWARD) return FORWARD;
    else return INVALID_INFO;
  } else if (lastNPassedSuperLayersOfStartSegment > firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == BACKWARD and endFBInfo == BACKWARD) return BACKWARD;
    else return INVALID_INFO;
  }

  // Now we are in the same passed super layer with both segments
  Index lastInTrackIdOfStartSegment = getLastInTrackId(ptrStartSegment2D);
  if (lastInTrackIdOfStartSegment == INVALID_INDEX) return INVALID_INFO;

  Index firstInTrackIdOfEndSegment = getFirstInTrackId(ptrEndSegment2D);
  if (firstInTrackIdOfEndSegment == INVALID_INDEX) return INVALID_INFO;

  if (startFBInfo == FORWARD and endFBInfo == FORWARD) {
    if (lastInTrackIdOfStartSegment < firstInTrackIdOfEndSegment) return FORWARD;
    else return INVALID_INFO;
  } else if (startFBInfo == BACKWARD and endFBInfo == BACKWARD) {
    // Test if end segment lies before in the mc track
    // Hence the whole pair of segments is reverse to the track direction of flight
    if (lastInTrackIdOfStartSegment > firstInTrackIdOfEndSegment) return BACKWARD;
    else return INVALID_INFO;
  }
  return INVALID_INFO;

}





CDCTrajectory3D CDCMCSegmentLookUp::getTrajectory3D(const CDCRecoSegment2D* ptrSegment2D) const
{
  CDCTrajectory3D trajectory3D;

  if (not ptrSegment2D) {
    B2WARNING("Segment is nullptr. Could not get fit.");
    return trajectory3D;
  }

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCHit* ptrFirstHit = getFirstHit(ptrSegment2D);
  const CDCSimHit* ptrPrimarySimHit = mcHitLookUp.getClosestPrimarySimHit(ptrFirstHit);

  if (not ptrPrimarySimHit) {
    // If there is no primary SimHit simply use the secondary simhit as reference
    ptrPrimarySimHit = mcHitLookUp.getSimHit(ptrFirstHit);
    if (not ptrPrimarySimHit) {
      B2WARNING("First simhit of CDCRecoSegment is nullptr. Could not get fit.");
      return trajectory3D;
    }
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  Vector3D mom3D = primarySimHit.getMomentum();
  Vector3D pos3D = primarySimHit.getPosTrack();

  int pdgCode = primarySimHit.getPDGCode();
  const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);

  if (not ptrTPDGParticle) {
    B2WARNING("No particle for PDG code " << pdgCode << ". Could not get fit");
    return trajectory3D;
  }

  const TParticlePDG& tPDGParticle = *ptrTPDGParticle;

  double charge = tPDGParticle.Charge() / 3.0;

  SignType chargeSign = sign(charge);

  trajectory3D.setPosMom3D(pos3D, mom3D, charge);

  SignType settedChargeSign = trajectory3D.getChargeSign();

  if (chargeSign != settedChargeSign) {
    B2WARNING("Charge sign of mc particle is not the same as the one of the fit");
  }

  return trajectory3D;
}
